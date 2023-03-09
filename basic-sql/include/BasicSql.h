/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _BASIC_SQL_H_
#define _BASIC_SQL_H_
#include "ConstStringSlice.h"
#include "SmallString.h"
#include "SmallVec.h"
#include "SqlDatabase.h"
#include "SqlError.h"
#include "SqlFile.h"
#include "SqlIndexFile.h"
#include "SqlParser.h"
#include "SqlTableFile.h"
#include "SqlTokenizer.h"
#include "SqlValue.h"

#include <string>

using basic_sql::SmallString;
using basic_sql::SmallVec;
using basic_sql::SqlDatabase;
using basic_sql::SqlError;
using basic_sql::SqlErrorType;
using basic_sql::SqlFile;
using basic_sql::SqlIndexFile;

// Temp
using basic_sql::COLUMN_MAX;
using basic_sql::COLUMN_NAME_MAX_LENGTH;
using basic_sql::DATABASE_MAX_NAME_SIZE;
using basic_sql::SqlError;
using basic_sql::TABLE_NAME_MAX_LENGTH;
using basic_sql::parser::SqlColumn;
using basic_sql::parser::SqlParser;
using basic_sql::parser::SqlParserError;
using basic_sql::parser::SqlStatement;
using basic_sql::parser::SqlStatementAlter;
using basic_sql::parser::SqlStatementCreateDatabase;
using basic_sql::parser::SqlStatementCreateTable;
using basic_sql::parser::SqlStatementDropDatabase;
using basic_sql::parser::SqlStatementDropTable;
using basic_sql::parser::SqlStatementSelect;
using basic_sql::parser::SqlStatementType;
using basic_sql::parser::SqlStatementUseDatabase;
using basic_sql::tokenizer::SqlTokenizerError;

namespace basic_sql {
using parser::SqlType;
using tokenizer::SqlToken;
using tokenizer::SqlTokenizer;
using tokenizer::SqlTokenizerError;
} // namespace basic_sql

/// A manager for sql databases
class SqlDatabaseManager {
public:
  SqlDatabaseManager() : current_database_name("") {}

  /// Load a db, without creating it
  void load_database(std::string name, SqlError error) {
    SqlDatabase database(name);
    bool create = false;
    database.open(create, error);
    if (!error.is_ok())
      return;
    databases[name] = std::move(database);
  }

  /// Create a database.
  void create(std::string name, SqlError &error) {
    auto database_it = this->databases.find(name);
    if (database_it != this->databases.end()) {
      error.set_already_exists();
      return;
    }

    // Check name length
    if (name.size() > DATABASE_MAX_NAME_SIZE) {
      error.set_limit_reached();
      return;
    }

    SqlDatabase database(name);
    bool create = true;
    database.open(create, error);
    if (error.type() == SqlErrorType::AlreadyExists) {
      bool create = false;
      database.open(create, error);
    }
    if (!error.is_ok())
      return;

    this->databases[name] = std::move(database);
  }

  /// Remove a db.
  void remove(std::string name, SqlError &error) {
    auto database_it = this->databases.find(name);
    if (database_it == this->databases.end()) {
      // this->load_database(name, error);
      // if (!error.is_ok())
      error.set_missing();
      return;
    }
    // refresh iter. If we got this far, we know for a fact that the db loaded
    // and is in the hash map; this cannot fail.
    database_it = this->databases.find(name);
    database_it->second.remove_database(error);
    databases.erase(database_it);
  }

  /// Try to use a db.
  void use(std::string name, SqlError &error) {
    if (this->databases.find(name) == this->databases.end()) {
      this->load_database(name, error);
      if (!error.is_ok())
        return;
    }

    current_database_name = name;
  }

  /// Try to create a table on the current db.
  void create_table(
      std::string name,
      const basic_sql::SmallVec<COLUMN_MAX, basic_sql::parser::SqlColumn>
          columns,
      SqlError &error) {
    if (this->current_database_name.size() == 0) {
      error.set_missing();
      return;
    }

    databases[this->current_database_name].create_table(name, columns, error);
  }

  /// Remove a table
  void remove_table(std::string name, SqlError &error) {
    if (this->current_database_name.size() == 0) {
      error.set_missing();
      return;
    }

    databases[this->current_database_name].remove_table(name, error);
  }

  /// Run a select statement
  void run_select_statement(basic_sql::parser::SqlStatementSelect &statement,
                            basic_sql::QueryRowsResult &result,
                            SqlError &error) {
    if (this->current_database_name.size() == 0) {
      error.set_missing();
      return;
    }

    databases[this->current_database_name].run_select_statement(statement,
                                                                result, error);
  }

  /// Run an alter statement
  void run_alter_statement(basic_sql::parser::SqlStatementAlter &statement,
                           SqlError &error) {
    if (this->current_database_name.size() == 0) {
      error.set_missing();
      return;
    }

    databases[this->current_database_name].run_alter_statement(statement,
                                                               error);
  }

  /// Run an insert statement.
  void run_insert_statement(basic_sql::parser::SqlStatementInsert &statement,
                            SqlError &error) {
    if (this->current_database_name.size() == 0) {
      error.set_missing();
      return;
    }

    databases[this->current_database_name].run_insert_statement(statement,
                                                                error);
  }

  /// Run an update statement.
  void run_update_statement(basic_sql::parser::SqlStatementUpdate &statement,
                            size_t &num_modified, SqlError &error) {
    if (this->current_database_name.size() == 0) {
      error.set_missing();
      return;
    }

    databases[this->current_database_name].run_update_statement(
        statement, num_modified, error);
  }

  /// Run a delete statement.
  void run_delete_statement(basic_sql::parser::SqlStatementDelete &statement,
                            size_t &num_modified, SqlError &error) {
    if (this->current_database_name.size() == 0) {
      error.set_missing();
      return;
    }

    databases[this->current_database_name].run_delete_statement(
        statement, num_modified, error);
  }

  /// Run a begin transaction statement
  void begin_transaction(SqlError &error) {
    if (this->current_database_name.size() == 0) {
      error.set_missing();
      return;
    }
    databases[this->current_database_name].begin_transaction();
  }

  /// Run a commit transaction statement
  void commit_transaction(SqlError &error) {
    if (this->current_database_name.size() == 0) {
      error.set_missing();
      return;
    }
    databases[this->current_database_name].commit_transaction(error);
  }

private:
  std::unordered_map<std::string, SqlDatabase> databases;
  /// The current db name.
  ///
  /// This is empty if there is no current database.
  std::string current_database_name;
};

#endif