/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SQL_DATABASE_H_
#define _SQL_DATABASE_H_

#include "Limits.h"
#include "SqlError.h"
#include "SqlIndexFile.h"
#include "SqlTableFile.h"
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>

namespace basic_sql {
class SqlDatabase {
public:
  // TODO: Remove
  /// This is invalid, but needed for unordered_map's [] accessor. This is very
  /// bug prone.
  SqlDatabase() : m_name("INVALID"), m_index(""), m_in_transaction(false) {}

  /// Make a new sql database
  ///
  /// name MUST be validated at this point.
  /// This does not open the db.
  SqlDatabase(std::string name)
      : m_name(name), m_index(name + "/index.db-index"),
        m_in_transaction(false), m_abort_transaction(false) {}
  SqlDatabase(const SqlDatabase &other) = delete;
  SqlDatabase &operator=(SqlDatabase &other) = delete;
  SqlDatabase(SqlDatabase &&other) noexcept
      : m_name(other.m_name), m_index(std::move(other.m_index)),
        tables(std::move(other.tables)),
        m_in_transaction(other.m_in_transaction),
        m_abort_transaction(other.m_abort_transaction) {}
  SqlDatabase &operator=(SqlDatabase &&other) {
    this->m_name = other.m_name;
    this->m_index = std::move(other.m_index);
    this->tables = std::move(other.tables);
    this->m_in_transaction = other.m_in_transaction;
    this->m_abort_transaction = other.m_abort_transaction;

    return *this;
  }

  /// Open or create a db
  void open(bool create, SqlError &error) {
    // Exit if already open
    if (!m_index.is_closed()) {
      error.set_file_already_opened();
      return;
    }

    // Check if dir exists
    struct stat dir_stat = {0};
    int dir_stat_error = stat(this->m_name.c_str(), &dir_stat);

    // If dir stat errors, aka it may not exist, process further.
    if (dir_stat_error == -1) {
      // If not a non exists error or we are creating a new db, exit
      if (errno == ENOENT && create) {
        // This is fine.
        // Create dir since it does not exist.
        int mkdir_error = mkdir(this->m_name.c_str(), 0700);
        if (mkdir_error != 0) {
          error.set_bad_mkdir();
          return;
        }
      } else {
        error.set_dir_stat();
        return;
      }
    }
    // Here dir exists and was created if it didn't.
    // TODO: maybe don't create dir if user doesnt want a new db.

    struct stat file_stat = {0};
    int file_stat_error = stat(this->m_index.file_name().c_str(), &file_stat);
    if (file_stat_error == -1) {
      // If not not exists error , exit
      if (errno != ENOENT) {
        error.set_dir_stat();
        return;
      } else {
        // if missing, but not creating db, exit.
        if (!create) {
          error.set_missing();
          return;
        }
      }
    } else {
      // If creating db, but it exists, exit.
      if (create) {
        error.set_already_exists();
        return;
      }
    }

    // Open index file
    this->m_index.open(create, error);
    if (!error.is_ok())
      return;

    // load entries
    size_t num_tables = this->m_index.get_num_tables();
    SmallString<TABLE_NAME_MAX_LENGTH> table_name;
    for (size_t i = 0; i < num_tables; i++) {
      this->m_index.get_table_name(i, table_name, error);
      if (!error.is_ok())
        return;
      std::string table_name_string(table_name.get_ptr(), table_name.size());
      std::string table_file_name;
      table_file_name += this->m_name + "/" + table_name_string + ".table";
      SqlTableFile table_file(table_file_name);
      bool create = false;
      table_file.open(create, error);
      if (!error.is_ok())
        return;
      this->tables.insert({table_name_string, std::move(table_file)});
    }
  }

  /// Remove this db
  void remove_database(SqlError &error) {
    this->close(error);
    if (!error.is_ok())
      return;
    this->m_index.remove_file(error);
    if (!error.is_ok())
      return;

    for (auto table_it = this->tables.begin(); table_it != this->tables.end();
         table_it++) {
      table_it->second.remove_file(error);
      if (!error.is_ok())
        return;
    }

    // TODO: handle errors
    assert(rmdir(m_name.c_str()) == 0);
  }

  /// Create a new table
  void create_table(std::string input_name,
                    const SmallVec<COLUMN_MAX, parser::SqlColumn> &columns,
                    SqlError &error) {
    // check for dupes in index
    SmallString<TABLE_NAME_MAX_LENGTH> name;
    name.append(input_name.c_str());
    if (this->m_index.index_of_table_name(name, error) != -1) {
      error.set_already_exists();
      return;
    }

    // create table file
    std::string table_file_name;
    table_file_name += this->m_name + "/" + input_name + ".table";
    SqlTableFile table_file(table_file_name);
    bool create = true;
    table_file.open(create, error);
    if (!error.is_ok())
      return;

    // insert columns
    for (size_t i = 0; i < columns.size(); i++) {
      table_file.add_column(columns[i], error);
      if (!error.is_ok())
        return;
    }

    // insert into memory tables
    this->tables.insert({input_name, std::move(table_file)});

    // insert into index
    name.clear();
    uint8_t num_tables = this->m_index.get_num_tables();
    name.append(input_name.c_str());
    this->m_index.insert_table_name(num_tables, name, error);
    if (!error.is_ok())
      return;
    this->m_index.update_num_tables(num_tables + 1, error);
    if (!error.is_ok())
      return;
  }

  /// Remove a table
  void remove_table(std::string input_name, SqlError &error) {
    SmallString<TABLE_NAME_MAX_LENGTH> table_name;
    table_name.append(input_name.c_str());

    // Locate index of name
    size_t index = this->m_index.index_of_table_name(table_name, error);
    if (!error.is_ok())
      return;
    if (index == -1) {
      error.set_missing();
      return;
    }

    // Locate index file
    auto table_it = this->tables.find(input_name);
    if (table_it == this->tables.end()) {
      panic("table present in index, missing in map");
      error.set_missing();
      return;
    }

    // Remove
    this->m_index.remove(index, error);
    if (!error.is_ok())
      return;

    // Remove table as well
    table_it->second.remove_file(error);
    if (!error.is_ok())
      return;
    this->tables.erase(table_it);
  }

  /// Run a select statement
  void run_select_statement(parser::SqlStatementSelect &statement,
                            QueryRowsResult &result,

                            SqlError &error) {
    // Locate index of name
    size_t index =
        this->m_index.index_of_table_name(statement.table_name, error);
    if (!error.is_ok())
      return;
    if (index == -1) {
      error.set_missing();
      return;
    }

    // fetch file
    std::string table_name(statement.table_name.get_ptr(),
                           statement.table_name.size());
    auto table_it = this->tables.find(table_name);
    if (table_it == this->tables.end()) {
      panic("select table present in index, missing in map");
      error.set_missing();
      return;
    }

    if (statement.join_type == parser::SqlJoinType::None) {
      const parser::SqlWhereClause *where_clause =
          statement.has_where_clause ? &statement.where_clause : nullptr;
      table_it->second.query_rows(statement.column_names, where_clause, result,
                                  error);
      if (!error.is_ok())
        return;
    } else {
      // Locate index of joined table name
      size_t joined_index =
          this->m_index.index_of_table_name(statement.joined_table_name, error);
      if (!error.is_ok())
        return;
      if (index == -1) {
        error.set_missing();
        return;
      }

      // fetch file
      std::string joined_table_name(statement.joined_table_name.get_ptr(),
                                    statement.joined_table_name.size());
      auto joined_table_it = this->tables.find(joined_table_name);
      if (joined_table_it == this->tables.end()) {
        panic("select joined table present in index, missing in map");
        error.set_missing();
        return;
      }

      // fetch first table
      QueryRowsResult first_result;
      table_it->second.query_rows(statement.column_names, nullptr, first_result,
                                  error);
      if (!error.is_ok())
        return;

      // fetch second table
      QueryRowsResult second_result;
      joined_table_it->second.query_rows(statement.column_names, nullptr,
                                         second_result, error);
      if (!error.is_ok())
        return;

      // get column indexes
      int first_column_index = table_it->second.get_index_of_column_name(
          statement.primary_join_column_name);
      assert(first_column_index != -1);
      int second_column_index =
          joined_table_it->second.get_index_of_column_name(
              statement.secondary_join_column_name);
      assert(second_column_index != -1);

      // build column name header
      for (size_t i = 0; i < table_it->second.get_columns().size(); i++) {
        result.columns.push(table_it->second.get_columns()[i]);
      }
      for (size_t i = 0; i < joined_table_it->second.get_columns().size();
           i++) {
        result.columns.push(joined_table_it->second.get_columns()[i]);
      }

      // join nested loop
      for (size_t first_result_index = 0;
           first_result_index < first_result.rows.size();
           first_result_index++) {
        bool left_matched = false;
        for (size_t second_result_index = 0;
             second_result_index < second_result.rows.size();
             second_result_index++) {

          // if eq, copy row. append joined columns to first table columns.
          bool join_match =
              first_result.rows[first_result_index][first_column_index] ==
              second_result.rows[second_result_index][second_column_index];
          if (join_match)

            if (join_match) {
              left_matched = true;

              SmallVec<COLUMN_MAX, SqlValue> row;
              for (size_t i = 0;
                   i < first_result.rows[first_result_index].size(); i++) {
                row.push(first_result.rows[first_result_index][i]);
              }
              for (size_t i = 0;
                   i < second_result.rows[second_result_index].size(); i++) {
                row.push(second_result.rows[second_result_index][i]);
              }

              result.rows.push_back(row);
            }
        }

        // if the left row didn't match, add it anyways and pad with nulls if
        // this is a left outer join
        if (statement.join_type == parser::SqlJoinType::LeftOuter &&
            !left_matched) {
          SmallVec<COLUMN_MAX, SqlValue> row;
          for (size_t i = 0; i < first_result.rows[first_result_index].size();
               i++) {
            row.push(first_result.rows[first_result_index][i]);
          }

          while (row.size() != result.columns.size()) {
            row.push(SqlValue());
          }
          result.rows.push_back(row);
        }
      }
    }
  }

  /// Run an alter statement
  void run_alter_statement(const parser::SqlStatementAlter &statement,
                           SqlError &error) {
    // Locate index of name
    size_t index =
        this->m_index.index_of_table_name(statement.table_name, error);
    if (!error.is_ok())
      return;
    if (index == -1) {
      error.set_missing();
      return;
    }

    std::string table_name(statement.table_name.get_ptr(),
                           statement.table_name.size());
    auto table_it = this->tables.find(table_name);
    if (table_it == this->tables.end()) {
      panic("alter table present in index, missing in map");
      error.set_missing();
      return;
    }
    table_it->second.add_column(statement.column, error);
    if (!error.is_ok())
      return;
  }

  /// Run an insert statement
  void run_insert_statement(const parser::SqlStatementInsert &statement,
                            SqlError &error) {
    // Locate index of name
    size_t index =
        this->m_index.index_of_table_name(statement.table_name, error);
    if (!error.is_ok())
      return;
    if (index == -1) {
      error.set_missing();
      return;
    }

    std::string table_name(statement.table_name.get_ptr(),
                           statement.table_name.size());
    auto table_it = this->tables.find(table_name);
    if (table_it == this->tables.end()) {
      panic("alter table present in index, missing in map");
      error.set_missing();
      return;
    }

    // get # of records
    uint8_t num_values = table_it->second.get_num_values();

    // insert record
    table_it->second.insert(num_values, statement.values, error);
    if (!error.is_ok())
      return;

    // increase # of values
    table_it->second.update_num_values(num_values + 1, error);
    if (!error.is_ok())
      return;
  }

  /// Run an update statement
  void run_update_statement(const parser::SqlStatementUpdate &statement,
                            size_t &num_modified, SqlError &error) {
    // Locate index of name
    size_t index =
        this->m_index.index_of_table_name(statement.table_name, error);
    if (!error.is_ok())
      return;
    if (index == -1) {
      error.set_missing();
      return;
    }

    std::string table_name(statement.table_name.get_ptr(),
                           statement.table_name.size());
    auto table_it = this->tables.find(table_name);
    if (table_it == this->tables.end()) {
      panic("alter table present in index, missing in map");
      error.set_missing();
      return;
    }

    // check if in txn
    if (this->m_in_transaction) {
      // create lock file string
      std::string lock_file_path = this->m_name + "/" + table_name + ".lock";

      // TODO: handle errors
      // Check if lock file exists
      struct stat stat_buf;
      int stat_code = stat(lock_file_path.c_str(), &stat_buf);
      if (stat_code == 0) {
        this->m_abort_transaction = true;
        error.set_file_already_opened();
        return;
      }

      // TODO: Return error
      // create lock file
      FILE *lock_file = fopen(lock_file_path.c_str(), "w");
      assert(lock_file != nullptr);
      assert(fclose(lock_file) == 0);

      this->m_locks.push_back(table_name);
    }

    // update
    table_it->second.update_rows(statement, this->m_in_transaction,
                                 num_modified, error);
    if (!error.is_ok()) {
      if (this->m_abort_transaction)
        this->m_abort_transaction = true;
      return;
    }
  }

  /// Run a delete statement
  void run_delete_statement(const parser::SqlStatementDelete &statement,
                            size_t &num_modified, SqlError &error) {
    // Locate index of name
    size_t index =
        this->m_index.index_of_table_name(statement.table_name, error);
    if (!error.is_ok())
      return;
    if (index == -1) {
      error.set_missing();
      return;
    }

    std::string table_name(statement.table_name.get_ptr(),
                           statement.table_name.size());
    auto table_it = this->tables.find(table_name);
    if (table_it == this->tables.end()) {
      panic("alter table present in index, missing in map");
      error.set_missing();
      return;
    }

    // delete
    table_it->second.delete_rows(statement, num_modified, error);
    if (!error.is_ok())
      return;
  }

  /// Begin a transaction
  void begin_transaction() { this->m_in_transaction = true; }

  /// commit a transaction
  void commit_transaction(SqlError &error) {
    this->m_in_transaction = false;

    if (this->m_abort_transaction)
      error.set_file_already_opened();

    for (size_t i = 0; i < this->m_locks.size(); i++) {
      if (!this->m_abort_transaction) {
        this->tables.find(this->m_locks[i])->second.commit(error);

        // remove lock
        // TODO: Check error
        std::string path = this->m_name + "/" + this->m_locks[i] + ".lock";
        assert(remove(path.c_str()) == 0);
      } else {
        this->tables.find(this->m_locks[i])->second.clear_buffered_rows();
      }
    }

    this->m_abort_transaction = false;
  }

  /// Close this db
  void close(SqlError &error) { this->m_index.close(error); }

protected:
private:
  std::string m_name;

  SqlIndexFile m_index;
  bool m_in_transaction;
  bool m_abort_transaction;
  std::vector<std::string> m_locks;
  std::unordered_map<std::string, SqlTableFile> tables;
};
} // namespace basic_sql
#endif