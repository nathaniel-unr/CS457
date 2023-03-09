/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#include "BasicSql.h"
#include <cassert>
#include <cctype>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

using basic_sql::SmallString;
using basic_sql::SqlError;
using basic_sql::SqlType;

using basic_sql::parser::SqlStatementInsert;

/// fmt a sql type to a stream
void fmt_sql_type(std::ostream &os, const SqlType &type) {
  switch (type.type) {
  case basic_sql::tokenizer::SqlType::INT:
    os << " int";
    break;
  case basic_sql::tokenizer::SqlType::VARCHAR:
    os << " varchar(" << type.size << ")";
    break;
  case basic_sql::tokenizer::SqlType::CHAR:
    os << " char(" << type.size << ")";
    break;
  case basic_sql::tokenizer::SqlType::FLOAT:
    os << " float";
    break;
  default:
    os << "Unknown SQL type (" << (int)type.type << ")" << std::endl;
    abort();
  }
}

int main() {
  // TODO: Encapsulate in DB Manager Class
  // TODO: make hashmap? this has a low # of dbs usually?
  // std::vector<SqlDatabase> databases;
  SmallString<DATABASE_MAX_NAME_SIZE> current_database_name;
  SqlDatabaseManager manager;

  bool should_exit = false;
  bool buffer_input = false;
  std::string string_input;
  while (!should_exit) {
    // clear flag or input buffer
    if (buffer_input) {
      buffer_input = false;
    } else {
      string_input.clear();
    }

    // std::cout << "-->";
    // TODO: handle errors?
    std::string temp_input;
    std::getline(std::cin, temp_input);
    string_input.append(temp_input);

    // TODO: Convert to string slice immediately, use for tokenizer
    // TODO: Add impl for from std::string construction
    // todo: why do i need the -1?
    basic_sql::ConstStringSlice string_input_slice(string_input.c_str(),
                                                   string_input.size() - 1);
    string_input_slice.trim();
    if (string_input_slice.size() == 0) {
      continue;
    }

    if (string_input_slice.case_insensitive_compare(".EXIT")) {
      should_exit = true;
      std::cout << "All done." << std::endl;
    } else if (string_input_slice.starts_with("--")) {
      // TODO: Lex and parse this instead of ignoring
      // ignore line
    } else {
      if (string_input.find(';') == std::string::npos) {
        buffer_input = true;
        continue;
      }

      SqlParser parser(string_input);
      std::vector<SqlStatement> statements;
      SqlParserError parser_error;
      parser.parse_all(statements, parser_error);
      basic_sql::parser::SqlParserErrorType parser_error_type =
          parser_error.type();
      switch (parser_error_type) {
      case basic_sql::parser::SqlParserErrorType::Ok:
        break;
      case basic_sql::parser::SqlParserErrorType::Tokenizer:
        std::cout << parser_error.get_tokenizer_error().message << std::endl;
        break;
      default:
        std::cout << "!Failed to parse statement. (" << (int)parser_error_type
                  << ")" << std::endl;
        break;
      }
      if (!parser_error.is_ok()) {
        continue;
      }

      // iter over each statement and execute each
      for (size_t i = 0; i < statements.size(); i++) {
        switch (statements[i].statement_type()) {
        case SqlStatementType::CREATE_DATABASE: {
          // process create database
          SqlStatementCreateDatabase &statement =
              statements[i].create_database();
          SqlError error;
          std::string database_name(statement.database_name.get_ptr(),
                                    statement.database_name.size());
          manager.create(database_name, error);

          basic_sql::SqlErrorType error_type = error.type();
          switch (error_type) {
          case SqlErrorType::Ok:
            std::cout << "Database " << database_name << " created."
                      << std::endl;
            break;
          case SqlErrorType::AlreadyExists: {
            std::cout << "!Failed to create database "
                      << statement.database_name
                      << " because it already exists." << std::endl;
            break;
          }
          default:
            std::cout << "!Failed to create database. (" << error_type << ")"
                      << std::endl;
            break;
          }

          break;
        }
        case SqlStatementType::DROP_DATABASE: {
          // process drop database
          SqlStatementDropDatabase &statement = statements[i].drop_database();
          SqlError error;
          std::string database_name(statement.database_name.get_ptr(),
                                    statement.database_name.size());
          manager.remove(database_name, error);

          SqlErrorType error_type = error.type();
          switch (error_type) {
          case SqlErrorType::Ok:
            std::cout << "Database " << statement.database_name << " deleted."
                      << std::endl;
            break;
          case SqlErrorType::Missing:
            std::cout << "!Failed to delete " << statement.database_name
                      << " because it does not exist." << std::endl;
            break;
          default:
            std::cout << "!Failed to delete database. (" << error_type << ")"
                      << std::endl;
            break;
          }
          break;
        }
        case SqlStatementType::USE_DATABASE: {
          // process use database
          SqlStatementUseDatabase &statement = statements[i].use_database();
          SqlError error;
          std::string database_name(statement.database_name.get_ptr(),
                                    statement.database_name.size());
          manager.use(database_name, error);
          SqlErrorType error_type = error.type();
          switch (error_type) {
          case SqlErrorType::Ok:
            std::cout << "Using database " << statement.database_name << "."
                      << std::endl;
            break;
          case SqlErrorType::Missing:
            std::cout << "!Database does not exist" << std::endl;
            break;
          default:
            std::cout << "!Failed to use database. (" << error_type << ")"
                      << std::endl;
            break;
          }
          break;
        }
        case SqlStatementType::CREATE_TABLE: {
          // process create table
          SqlStatementCreateTable &statement = statements[i].create_table();
          std::string table_name(statement.table_name.get_ptr(),
                                 statement.table_name.size());
          SqlError error;
          manager.create_table(table_name, statement.columns, error);
          SqlErrorType error_type = error.type();
          switch (error_type) {
          case SqlErrorType::Ok:
            std::cout << "Table " << statement.table_name << " created."
                      << std::endl;
            break;
          case SqlErrorType::AlreadyExists:
            std::cout << "!Failed to create " << statement.table_name
                      << " because it already exists." << std::endl;
            break;
          default:
            std::cout << "!Failed to create table. (" << error.type() << ")"
                      << std::endl;
            break;
          }
          break;
        }
        case SqlStatementType::DROP_TABLE: {
          // process drop table
          SqlStatementDropTable &statement = statements[i].drop_table();
          SqlError error;
          std::string table_name(statement.table_name.get_ptr(),
                                 statement.table_name.size());
          manager.remove_table(table_name, error);

          SqlErrorType error_type = error.type();
          switch (error_type) {
          case SqlErrorType::Ok:
            std::cout << "Table " << statement.table_name << " deleted."
                      << std::endl;
            break;
          case SqlErrorType::Missing:
            std::cout << "!Failed to delete " << statement.table_name
                      << " because it does not exist." << std::endl;
            break;
          default:
            std::cout << "!Failed to delete table. (" << error.type() << ")"
                      << std::endl;
            break;
          }
          break;
        }
        case SqlStatementType::SELECT: {
          // process select
          SqlStatementSelect &statement = statements[i].select();
          basic_sql::QueryRowsResult result;
          SqlError error;
          manager.run_select_statement(statement, result, error);

          // handle results
          SqlErrorType error_type = error.type();
          switch (error_type) {
          case SqlErrorType::Ok:
            for (size_t i = 0; i < result.columns.size(); i++) {
              std::cout << result.columns[i].name;
              fmt_sql_type(std::cout, result.columns[i].type);
              if (i + 1 < result.columns.size()) {
                std::cout << " | ";
              }
            }
            std::cout << std::endl;

            for (size_t row_index = 0; row_index != result.rows.size();
                 row_index++) {
              for (size_t i = 0; i < result.rows[row_index].size(); i++) {
                std::cout << result.rows[row_index][i];
                if (i + 1 < result.rows[row_index].size()) {
                  std::cout << " | ";
                }
              }
              std::cout << std::endl;
            }

            break;
          case SqlErrorType::Missing:
            std::cout << "!Failed to query table " << statement.table_name
                      << " because it does not exist." << std::endl;
            break;
          default:
            std::cout << "!Failed to select. (" << error.type() << ")"
                      << std::endl;
            break;
          }

          break;
        }
        case SqlStatementType::ALTER: {
          // process alter
          SqlStatementAlter &statement = statements[i].alter();
          SqlError error;
          manager.run_alter_statement(statement, error);

          // handle results
          SqlErrorType error_type = error.type();
          switch (error_type) {
          case SqlErrorType::Ok:
            std::cout << "Table " << statement.table_name << " modified."
                      << std::endl;
            break;
          default:
            std::cout << "!Failed to alter. (" << error.type() << ")"
                      << std::endl;
            break;
          }
          break;
        }
        case SqlStatementType::INSERT: {
          // process insert
          SqlStatementInsert &statement = statements[i].insert();
          SqlError error;
          manager.run_insert_statement(statement, error);

          // handle results
          SqlErrorType error_type = error.type();
          switch (error_type) {
          case SqlErrorType::Ok:
            std::cout << "1 new record inserted." << std::endl;
            break;
          default:
            std::cout << "!Failed to insert. (" << error.type() << ")"
                      << std::endl;
            break;
          }

          break;
        }
        case SqlStatementType::UPDATE: {
          // process update
          basic_sql::parser::SqlStatementUpdate &statement =
              statements[i].update();
          SqlError error;
          size_t num_modified = 0;
          manager.run_update_statement(statement, num_modified, error);

          // handle results
          SqlErrorType error_type = error.type();
          switch (error_type) {
          case SqlErrorType::Ok:
            if (num_modified == 1) {
              std::cout << "1 record modified." << std::endl;
            } else {
              std::cout << num_modified << " records modified." << std::endl;
            }

            break;
          case SqlErrorType::FileAlreadyOpened:
            std::cout << "Error: Table " << statement.table_name
                      << " is locked!" << std::endl;
            break;
          default:
            std::cout << "!Failed to update. (" << error.type() << ")"
                      << std::endl;
            break;
          }

          break;
        }
        case SqlStatementType::DELETE: {
          // process delete
          basic_sql::parser::SqlStatementDelete &statement =
              statements[i].get_delete();
          SqlError error;
          size_t num_deleted = 0;
          manager.run_delete_statement(statement, num_deleted, error);

          // handle results
          SqlErrorType error_type = error.type();
          switch (error_type) {
          case SqlErrorType::Ok:
            if (num_deleted == 1) {
              std::cout << "1 record deleted." << std::endl;
            } else {
              std::cout << num_deleted << " records deleted." << std::endl;
            }
            break;
          default:
            std::cout << "!Failed to delete. (" << error.type() << ")"
                      << std::endl;
            break;
          }
          break;
        }
        case SqlStatementType::BEGIN_TRANSACTION: {
          SqlError error;
          manager.begin_transaction(error);
          std::cout << "Transaction starts." << std::endl;
          break;
        }
        case SqlStatementType::COMMIT_TRANSACTION: {
          SqlError error;
          manager.commit_transaction(error);

          // handle results
          SqlErrorType error_type = error.type();
          switch (error_type) {
          case SqlErrorType::Ok: {
            std::cout << "Transaction committed." << std::endl;
            break;
          }
          case SqlErrorType::FileAlreadyOpened: {
            std::cout << "Transaction abort." << std::endl;
            break;
          }
          default:
            std::cout << "!Failed to commit transaction. (" << error.type()
                      << ")" << std::endl;
            break;
          }
          break;
        }
        default:
          std::cout << "Unknown statement type" << std::endl;
          break;
        }
      }
    }
  }

  return 0;
}