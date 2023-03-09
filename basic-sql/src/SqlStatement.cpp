/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#include "SqlStatement.h"
namespace basic_sql {
namespace parser {
/// Make a sql statement from a create database statement
SqlStatement::SqlStatement(SqlStatementCreateDatabase create_database)
    : m_statement_type(SqlStatementType::CREATE_DATABASE),
      m_create_database(create_database) {}
/// Make a sql statement from a drop database statement
SqlStatement::SqlStatement(SqlStatementDropDatabase drop_database)
    : m_statement_type(SqlStatementType::DROP_DATABASE),
      m_drop_database(drop_database) {}
/// Make a sql statement from a use database statement
SqlStatement::SqlStatement(SqlStatementUseDatabase use_database)
    : m_statement_type(SqlStatementType::USE_DATABASE),
      m_use_database(use_database) {}
/// Make a sql statement from a create table statement
SqlStatement::SqlStatement(SqlStatementCreateTable create_table)
    : m_statement_type(SqlStatementType::CREATE_TABLE),
      m_create_table(create_table) {}
/// Make a sql statement from a drop table statement
SqlStatement::SqlStatement(SqlStatementDropTable drop_table)
    : m_statement_type(SqlStatementType::DROP_TABLE), m_drop_table(drop_table) {
}
/// Make a sql statement from a select statement
SqlStatement::SqlStatement(SqlStatementSelect select)
    : m_statement_type(SqlStatementType::SELECT), m_select(select) {}
/// Make a sql statement from an alter statement
SqlStatement::SqlStatement(SqlStatementAlter alter)
    : m_statement_type(SqlStatementType::ALTER), m_alter(alter) {}
/// Make a sql statement from an insert statement
SqlStatement::SqlStatement(SqlStatementInsert insert)
    : m_statement_type(SqlStatementType::INSERT), m_insert(insert) {}
/// Make a sql statement from an update statement
SqlStatement::SqlStatement(SqlStatementUpdate update)
    : m_statement_type(SqlStatementType::UPDATE), m_update(update) {}
/// Make a sql statement from a delete statement
SqlStatement::SqlStatement(SqlStatementDelete del)
    : m_statement_type(SqlStatementType::DELETE), m_delete(del) {}
/// Make a sql statement from a begin transaction statement
SqlStatement::SqlStatement(SqlStatementBeginTransaction begin_transaction)
    : m_statement_type(SqlStatementType::BEGIN_TRANSACTION),
      m_begin_transaction(begin_transaction) {}
/// Make a sql statement from a commit transaction statement
SqlStatement::SqlStatement(SqlStatementCommitTransaction commit_transaction)
    : m_statement_type(SqlStatementType::COMMIT_TRANSACTION),
      m_commit_transaction(commit_transaction) {}
/// Copy constructor
SqlStatement::SqlStatement(const SqlStatement &other) {
  this->m_statement_type = other.m_statement_type;
  switch (m_statement_type) {
  case SqlStatementType::CREATE_DATABASE:
    this->m_create_database = other.m_create_database;
    break;
  case SqlStatementType::DROP_DATABASE:
    this->m_drop_database = other.m_drop_database;
    break;
  case SqlStatementType::USE_DATABASE:
    this->m_use_database = other.m_use_database;
    break;
  case SqlStatementType::CREATE_TABLE:
    this->m_create_table = other.m_create_table;
    break;
  case SqlStatementType::DROP_TABLE:
    this->m_drop_table = other.m_drop_table;
    break;
  case SqlStatementType::SELECT:
    this->m_select = other.m_select;
    break;
  case SqlStatementType::ALTER:
    this->m_alter = other.m_alter;
    break;
  case SqlStatementType::INSERT:
    this->m_insert = other.m_insert;
    break;
  case SqlStatementType::UPDATE:
    this->m_update = other.m_update;
    break;
  case SqlStatementType::DELETE:
    this->m_delete = other.m_delete;
    break;
  case SqlStatementType::BEGIN_TRANSACTION:
    this->m_begin_transaction = other.m_begin_transaction;
    break;
  case SqlStatementType::COMMIT_TRANSACTION:
    this->m_commit_transaction = other.m_commit_transaction;
    break;
  default:
    panic("unknown sqlstatement type in copy constructor");
  }
}
/// Get the statement type
SqlStatementType SqlStatement::statement_type() {
  return this->m_statement_type;
}
/// Get the create database statement
///
/// This must contain a create database statement.
SqlStatementCreateDatabase &SqlStatement::create_database() {
  assert(this->m_statement_type == SqlStatementType::CREATE_DATABASE);
  return this->m_create_database;
}
/// Get the drop database statement
///
/// This must contain a drop database statement.
SqlStatementDropDatabase &SqlStatement::drop_database() {
  assert(this->m_statement_type == SqlStatementType::DROP_DATABASE);
  return this->m_drop_database;
}
/// Get the use database statement
///
/// This must contain a use database statement.
SqlStatementUseDatabase &SqlStatement::use_database() {
  assert(this->m_statement_type == SqlStatementType::USE_DATABASE);
  return this->m_use_database;
}
/// Get the create table statement
///
/// This must contain a create table statement
SqlStatementCreateTable &SqlStatement::create_table() {
  assert(this->m_statement_type == SqlStatementType::CREATE_TABLE);
  return this->m_create_table;
}
/// Get the drop table statement
///
/// This must contain a drop table statement
SqlStatementDropTable &SqlStatement::drop_table() {
  assert(this->m_statement_type == SqlStatementType::DROP_TABLE);
  return this->m_drop_table;
}
/// Get the select statement
///
/// This must contain a select statement
SqlStatementSelect &SqlStatement::select() {
  assert(this->m_statement_type == SqlStatementType::SELECT);
  return this->m_select;
}
/// Get the alter statement
///
/// This must contain an alter statement
SqlStatementAlter &SqlStatement::alter() {
  assert(this->m_statement_type == SqlStatementType::ALTER);
  return this->m_alter;
}
/// Get the insert statement
///
/// This must contain an insert statement
SqlStatementInsert &SqlStatement::insert() {
  assert(this->m_statement_type == SqlStatementType::INSERT);
  return this->m_insert;
}
/// Get the update statement
///
/// This must contain an update statement
SqlStatementUpdate &SqlStatement::update() {
  assert(this->m_statement_type == SqlStatementType::UPDATE);
  return this->m_update;
}
/// Get the delete statement
///
/// This must contain an delete statement
SqlStatementDelete &SqlStatement::get_delete() {
  assert(this->m_statement_type == SqlStatementType::DELETE);
  return this->m_delete;
}
} // namespace parser
} // namespace basic_sql