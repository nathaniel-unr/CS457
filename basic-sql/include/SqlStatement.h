/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SQL_STATEMENT_H_
#define _SQL_STATEMENT_H_

#include "Limits.h"
#include "SmallString.h"
#include "SmallVec.h"
#include "SqlColumn.h"
#include "SqlToken.h"
#include "SqlValue.h"
#include "parser/SqlType.h"

namespace basic_sql {
namespace parser {

/// a table join type
enum class SqlJoinType {
  None,
  Inner,
  LeftOuter,
};

/// a where clause
struct SqlWhereClause {
  /// the column name
  SmallString<COLUMN_NAME_MAX_LENGTH> column_name;
  /// the op
  tokenizer::SqlOperator op;
  /// the value
  SqlValue value;

  /// check if a value matches this clause.
  ///
  /// This does not validate the column name
  bool value_matches(const SqlValue &other) const {
    switch (this->op) {
    case tokenizer::SqlOperator::Equals:
      return other == this->value;
    case tokenizer::SqlOperator::GreaterThan:
      return other > this->value;
    case tokenizer::SqlOperator::NotEqual:
      return other != this->value;
    default:
      panic("unknown op in `bool value_matches(const SqlValue &other)`");
      return false;
    }
  }
};

/// A create database statement
struct SqlStatementCreateDatabase {
  /// The database name
  SmallString<DATABASE_MAX_NAME_SIZE> database_name;
};

/// A drop database statement
struct SqlStatementDropDatabase {
  /// The database name
  SmallString<DATABASE_MAX_NAME_SIZE> database_name;
};

/// A use database statement
struct SqlStatementUseDatabase {
  /// The database name
  SmallString<DATABASE_MAX_NAME_SIZE> database_name;
};

/// A create table statement
struct SqlStatementCreateTable {
  /// The table name
  SmallString<TABLE_NAME_MAX_LENGTH> table_name;

  /// The columns in the table
  SmallVec<COLUMN_MAX, SqlColumn> columns;
};

/// A drop table statement
struct SqlStatementDropTable {
  /// The name of the table
  SmallString<TABLE_NAME_MAX_LENGTH> table_name;
};

/// A select statement
struct SqlStatementSelect {
  /// The table name
  SmallString<TABLE_NAME_MAX_LENGTH> table_name;
  /// column names
  ///
  /// This is empty if the user requested all columns
  SmallVec<COLUMN_MAX, SmallString<COLUMN_NAME_MAX_LENGTH>> column_names;
  /// True if the where clause is valid
  bool has_where_clause;
  /// The where clause
  ///
  /// only valid if has_where_clause is true
  SqlWhereClause where_clause;

  /// The join type
  SqlJoinType join_type;

  /// The joined table name
  SmallString<TABLE_NAME_MAX_LENGTH> joined_table_name;

  /// The main table's joined column name
  SmallString<COLUMN_NAME_MAX_LENGTH> primary_join_column_name;

  /// The joined table's joined column name
  SmallString<COLUMN_NAME_MAX_LENGTH> secondary_join_column_name;
};

/// An alter statement
struct SqlStatementAlter {
  /// The table to alter
  SmallString<TABLE_NAME_MAX_LENGTH> table_name;
  /// The new column
  SqlColumn column;
};

/// An insert statement
struct SqlStatementInsert {
  /// The table to insert
  SmallString<TABLE_NAME_MAX_LENGTH> table_name;

  /// The values
  SmallVec<COLUMN_MAX, SqlValue> values;
};

/// an update statement
struct SqlStatementUpdate {
  /// The table to update
  SmallString<TABLE_NAME_MAX_LENGTH> table_name;

  /// the column to update
  SmallString<TABLE_NAME_MAX_LENGTH> column_name;

  /// the new column value
  SqlValue value;

  /// the where clause
  SqlWhereClause where_clause;
};

/// a delete statement
struct SqlStatementDelete {
  /// The table to update
  SmallString<TABLE_NAME_MAX_LENGTH> table_name;

  /// the where clause
  SqlWhereClause where_clause;
};

/// A begin transaction statement
struct SqlStatementBeginTransaction {};

/// A commit transaction statement
struct SqlStatementCommitTransaction {};

/// The sql statement type
enum class SqlStatementType {
  CREATE_DATABASE,
  DROP_DATABASE,
  USE_DATABASE,
  CREATE_TABLE,
  DROP_TABLE,
  SELECT,
  ALTER,
  INSERT,
  UPDATE,
  DELETE,
  BEGIN_TRANSACTION,
  COMMIT_TRANSACTION,
};

/// A sql statement
class SqlStatement {
public:
  /// Make a sql statement from a create database statement
  SqlStatement(SqlStatementCreateDatabase create_database);
  /// Make a sql statement from a drop database statement
  SqlStatement(SqlStatementDropDatabase drop_database);
  /// Make a sql statement from a use database statement
  SqlStatement(SqlStatementUseDatabase use_database);
  /// Make a sql statement from a create table statement
  SqlStatement(SqlStatementCreateTable create_table);
  /// Make a sql statement from a drop table statement
  SqlStatement(SqlStatementDropTable drop_table);
  /// Make a sql statement from a select statement
  SqlStatement(SqlStatementSelect select);
  /// Make a sql statement from an alter statement
  SqlStatement(SqlStatementAlter alter);
  /// Make a sql statement from an insert statement
  SqlStatement(SqlStatementInsert insert);
  /// Make a sql statement from an update statement
  SqlStatement(SqlStatementUpdate update);
  /// Make a sql statement from an delete statement
  SqlStatement(SqlStatementDelete del);
  /// Make a sql statement from a begin transaction statement
  SqlStatement(SqlStatementBeginTransaction begin_transaction);
  /// Make a sql statement from a commit transaction statement
  SqlStatement(SqlStatementCommitTransaction commit_transaction);
  /// Copy constructor
  SqlStatement(const SqlStatement &other);
  /// Get the statement type
  SqlStatementType statement_type();
  /// Get the create database statement
  ///
  /// This must contain a create database statement.
  SqlStatementCreateDatabase &create_database();
  /// Get the drop database statement
  ///
  /// This must contain a drop database statement.
  SqlStatementDropDatabase &drop_database();
  /// Get the use database statement
  ///
  /// This must contain a use database statement.
  SqlStatementUseDatabase &use_database();
  /// Get the create table statement
  ///
  /// This must contain a create table statement
  SqlStatementCreateTable &create_table();
  /// Get the drop table statement
  ///
  /// This must contain a drop table statement
  SqlStatementDropTable &drop_table();
  /// Get the select statement
  ///
  /// This must contain a select statement
  SqlStatementSelect &select();
  /// Get the alter statement
  ///
  /// This must contain an alter statement
  SqlStatementAlter &alter();
  /// Get the insert statement
  ///
  /// This must contain an insert statement.
  SqlStatementInsert &insert();
  /// Get the update statement
  ///
  /// This must contain an update statement.
  SqlStatementUpdate &update();
  /// Get the delete statement
  ///
  /// This must contain an delete statement
  SqlStatementDelete &get_delete();

protected:
private:
  SqlStatementType m_statement_type;
  union {
    SqlStatementCreateDatabase m_create_database;
    SqlStatementDropDatabase m_drop_database;
    SqlStatementUseDatabase m_use_database;
    SqlStatementCreateTable m_create_table;
    SqlStatementDropTable m_drop_table;
    SqlStatementSelect m_select;
    SqlStatementAlter m_alter;
    SqlStatementInsert m_insert;
    SqlStatementUpdate m_update;
    SqlStatementDelete m_delete;
    SqlStatementBeginTransaction m_begin_transaction;
    SqlStatementCommitTransaction m_commit_transaction;
  };
};
} // namespace parser
} // namespace basic_sql
#endif