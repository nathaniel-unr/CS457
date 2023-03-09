/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#include "SqlParser.h"

namespace basic_sql {
namespace parser {

/// Make a parser error that is ok.
SqlParserError::SqlParserError() : m_error_type(SqlParserErrorType::Ok) {}
/// Set this to a tokenizer error
void SqlParserError::set_tokenizer_error(
    tokenizer::SqlTokenizerError tokenizer_error) {
  this->m_error_type = SqlParserErrorType::Tokenizer;
  this->m_tokenizer_error = tokenizer_error;
}
/// Set the error to be an unexpected end.
void SqlParserError::set_unexpected_end() {
  this->m_error_type = SqlParserErrorType::UnexpectedEnd;
}
/// Set the error to be an unexpected token.
void SqlParserError::set_unexpected_token(tokenizer::SqlTokenType token_type) {
  this->m_error_type = SqlParserErrorType::UnexpectedToken;
  this->m_unexpected_token_type = token_type;
}
/// Set the error to be an limit reached
void SqlParserError::set_limit_reached() {
  this->m_error_type = SqlParserErrorType::LimitReached;
}
/// Check if this error is ok.
bool SqlParserError::is_ok() const {
  return this->m_error_type == SqlParserErrorType::Ok;
}

// TODO: maybe just proivde the token buffer, and a utility func to tokenize
// the string and feed it
// TODO: Consider parser reuse
/// Make a new parser from the given input
SqlParser::SqlParser(const std::string &input)
    : tokenizer(input), position(0) {}
/// Parse all statements into a vec.
void SqlParser::parse_all(
    std::vector<basic_sql::parser::SqlStatement> &statements,
    SqlParserError &error) {
  tokenizer::SqlTokenizerError tokenizer_error;
  this->tokenizer.tokenize(this->tokens, tokenizer_error);
  if (!tokenizer_error.is_ok()) {
    error.set_tokenizer_error(tokenizer_error);
    return;
  }

  while (this->peek() != nullptr) {
    this->parse_statement(statements, error);
    if (!error.is_ok())
      return;
  }
}
/// Parse a single statement
void SqlParser::parse_statement(std::vector<SqlStatement> &statements,
                                SqlParserError &error) {
  const tokenizer::SqlToken *token = this->peek();
  if (token == nullptr) {
    error.set_unexpected_end();
    return;
  }

  switch (token->token_type()) {
  case tokenizer::SqlTokenType::KEYWORD:
    switch (token->keyword()) {
    case tokenizer::SqlKeyword::CREATE: {
      // consume token
      this->read();

      const tokenizer::SqlKeyword *keyword = nullptr;
      this->read_keyword(&keyword, error);
      if (!error.is_ok())
        return;

      switch (*keyword) {
      case tokenizer::SqlKeyword::DATABASE: {
        // CREATE DATABASE <identifier>;
        const tokenizer::SqlIdentifier *identifier = nullptr;
        this->read_identifier(&identifier, error);
        if (!error.is_ok())
          return;
        if (identifier->value.size() > DATABASE_MAX_NAME_SIZE) {
          error.set_limit_reached();
          return;
        }
        this->read_semicolon(error);
        if (!error.is_ok())
          return;

        SmallString<DATABASE_MAX_NAME_SIZE> database_name(
            identifier->value.get_ptr(), identifier->value.size());
        SqlStatementCreateDatabase create_database_statement{
            database_name,
        };
        SqlStatement statement(create_database_statement);
        statements.push_back(statement);
        break;
      }

      case tokenizer::SqlKeyword::TABLE: {
        // CREATE TABLE <identifier> (a1 int, a2 varchar(20));

        // read table name
        SmallString<TABLE_NAME_MAX_LENGTH> table_name;
        this->read_table_name(table_name, error);
        if (!error.is_ok())
          return;

        // read (
        this->read_left_parenthesis(error);
        if (!error.is_ok())
          return;

        SmallVec<COLUMN_MAX, SqlColumn> columns;

        // Ensure at least 1 column
        {
          const tokenizer::SqlIdentifier *column_identifier = nullptr;
          this->read_identifier(&column_identifier, error);
          if (!error.is_ok())
            return;
          if (column_identifier->value.size() > COLUMN_NAME_MAX_LENGTH) {
            error.set_limit_reached();
            return;
          }
          SqlType type;
          this->read_type(&type, error);
          if (!error.is_ok())
            return;

          SmallString<COLUMN_NAME_MAX_LENGTH> name(
              column_identifier->value.get_ptr(),
              column_identifier->value.size());
          columns.push(SqlColumn{
            name : name,
            type : type,
          });
        }

        // Handle more than 1 column
        const tokenizer::SqlToken *token = this->peek();
        while (token != nullptr &&
               token->token_type() == tokenizer::SqlTokenType::COMMA) {
          // We already validate the comma above, so just pop it from the
          // buffer.
          this->read();

          const tokenizer::SqlIdentifier *column_identifier = nullptr;
          this->read_identifier(&column_identifier, error);
          if (!error.is_ok())
            return;
          if (column_identifier->value.size() > COLUMN_NAME_MAX_LENGTH) {
            error.set_limit_reached();
            return;
          }
          SqlType type;
          this->read_type(&type, error);
          if (!error.is_ok())
            return;

          SmallString<COLUMN_NAME_MAX_LENGTH> name(
              column_identifier->value.get_ptr(),
              column_identifier->value.size());
          columns.push(SqlColumn{
            name : name,
            type : type,
          });

          token = this->peek();
        }

        // read )
        this->read_right_parenthesis(error);
        if (!error.is_ok())
          return;

        // read ;
        this->read_semicolon(error);
        if (!error.is_ok())
          return;

        statements.push_back(
            SqlStatement(SqlStatementCreateTable{table_name, columns}));
        break;
      }

      default:
        // TODO: Make unexpected keyword error or upgrade unexpected token error
        std::cout << "UNEXPECTED KEYWORD IN CREATE: " << keyword << std::endl;
        abort();
      }

      break;
    }
    case tokenizer::SqlKeyword::DROP: {
      // consume token
      this->read();

      const tokenizer::SqlKeyword *keyword = nullptr;
      this->read_keyword(&keyword, error);
      if (!error.is_ok())
        return;

      switch (*keyword) {
      case tokenizer::SqlKeyword::DATABASE: {
        // DROP DATABASE <identifier>;
        const tokenizer::SqlIdentifier *identifier = nullptr;
        this->read_identifier(&identifier, error);
        if (!error.is_ok())
          return;
        if (identifier->value.size() > DATABASE_MAX_NAME_SIZE) {
          error.set_limit_reached();
          return;
        }
        this->read_semicolon(error);
        if (!error.is_ok())
          return;

        SqlStatementDropDatabase drop_database_statement{
          database_name : SmallString<DATABASE_MAX_NAME_SIZE>(
              identifier->value.get_ptr(), identifier->value.size())
        };
        SqlStatement statement(drop_database_statement);
        statements.push_back(statement);
        break;
      }
      case tokenizer::SqlKeyword::TABLE: {
        // DROP TABLE <identifier>;
        // read table name

        // read table name
        SmallString<TABLE_NAME_MAX_LENGTH> table_name;
        this->read_table_name(table_name, error);
        if (!error.is_ok())
          return;
        this->read_semicolon(error);
        if (!error.is_ok())
          return;

        SqlStatementDropTable drop_table_statement{table_name};
        SqlStatement statement(drop_table_statement);
        statements.push_back(statement);
        break;
      }
      default:
        // TODO: Make unexpected keyword error or upgrade unexpected token error
        std::cout << "UNEXPECTED KEYWORD IN DROP: " << keyword << std::endl;
        abort();
      }
      break;
    }
    case tokenizer::SqlKeyword::USE: {
      // consume token
      this->read();

      // USE <identifier>;
      const tokenizer::SqlIdentifier *identifier = nullptr;
      this->read_identifier(&identifier, error);
      if (!error.is_ok())
        return;
      if (identifier->value.size() > TABLE_NAME_MAX_LENGTH) {
        error.set_limit_reached();
        return;
      }
      this->read_semicolon(error);
      if (!error.is_ok())
        return;

      // Return
      SmallString<DATABASE_MAX_NAME_SIZE> database_name(
          identifier->value.get_ptr(), identifier->value.size());
      SqlStatementUseDatabase use_database_statement{
          database_name,
      };
      SqlStatement statement(use_database_statement);
      statements.push_back(statement);
      break;
    }
    case tokenizer::SqlKeyword::SELECT: {
      // consume select token
      this->read();

      // SELECT * FROM <table>;
      // select name, price from product where pid != 2;
      // select * from Employee E, Sales S where E.id = S.employeeID;

      // parse asterisk or columns
      if (!this->has_input()) {
        error.set_unexpected_end();
        return;
      }
      tokenizer::SqlTokenType peek_token_type = this->peek()->token_type();
      SmallVec<COLUMN_MAX, SmallString<COLUMN_NAME_MAX_LENGTH>> column_names;
      if (peek_token_type == tokenizer::SqlTokenType::ASTERISK) {
        // consume asterisk
        this->read();
      } else if (peek_token_type == tokenizer::SqlTokenType::IDENTIFIER) {
        // read column name
        SmallString<COLUMN_NAME_MAX_LENGTH> column_name;
        this->read_column_name(column_name, error);
        if (!error.is_ok())
          return;

        column_names.push(column_name);

        // peek next type
        if (!this->has_input()) {
          error.set_unexpected_end();
          return;
        }
        peek_token_type = this->peek()->token_type();

        while (peek_token_type == tokenizer::SqlTokenType::COMMA) {
          // consume comma
          this->read();

          // read column name
          this->read_column_name(column_name, error);
          if (!error.is_ok())
            return;

          column_names.push(column_name);

          // peek next token type
          if (!this->has_input()) {
            error.set_unexpected_end();
            return;
          }
          peek_token_type = this->peek()->token_type();
        }
      }

      // TODO: validate from
      this->read();

      // read table name
      SmallString<TABLE_NAME_MAX_LENGTH> table_name;
      this->read_table_name(table_name, error);
      if (!error.is_ok())
        return;

      // parse table alias
      if (!this->has_input()) {
        error.set_unexpected_end();
        return;
      }
      SmallString<TABLE_NAME_MAX_LENGTH> table_name_alias;
      if (this->peek()->token_type() == tokenizer::SqlTokenType::IDENTIFIER) {
        this->read_table_name(table_name_alias, error);
        if (!error.is_ok())
          return;
      }

      // TODO: Support more than 1 table
      SqlJoinType join_type = SqlJoinType::None;
      SmallString<TABLE_NAME_MAX_LENGTH> joined_table_name;
      SmallString<TABLE_NAME_MAX_LENGTH> joined_table_name_alias;
      if (this->peek()->token_type() == tokenizer::SqlTokenType::COMMA) {
        // consume comma
        this->read();

        // read joined table name
        this->read_table_name(joined_table_name, error);
        if (!error.is_ok())
          return;

        // TODO: allow missing alias
        // read joined table name alias
        this->read_table_name(joined_table_name_alias, error);
        if (!error.is_ok())
          return;

        join_type = SqlJoinType::Inner;
      }

      if (!this->has_input()) {
        error.set_unexpected_end();
        return;
      }

      if (*this->peek() == tokenizer::SqlToken(tokenizer::SqlKeyword::INNER)) {
        // consume "INNER"
        this->read();

        // TODO: validate "JOIN"
        this->read();

        // read joined table name
        this->read_table_name(joined_table_name, error);
        if (!error.is_ok())
          return;

        // TODO: allow missing alias
        // read joined table name alias
        this->read_table_name(joined_table_name_alias, error);
        if (!error.is_ok())
          return;

        // TODO: validate "ON"
        this->read();

        join_type = SqlJoinType::Inner;
      } else if (*this->peek() ==
                 tokenizer::SqlToken(tokenizer::SqlKeyword::LEFT)) {
        // consume "LEFT"
        this->read();

        // TODO: validate "OUTER"
        this->read();

        // TODO: validate "JOIN"
        this->read();

        // read joined table name
        this->read_table_name(joined_table_name, error);
        if (!error.is_ok())
          return;

        // TODO: allow missing alias
        // read joined table name alias
        this->read_table_name(joined_table_name_alias, error);
        if (!error.is_ok())
          return;

        // TODO: validate "ON"
        this->read();

        join_type = SqlJoinType::LeftOuter;
      }

      // TODO: for simplicity, temporarily disallow wheres on joins.
      bool has_where_clause = false;
      SqlWhereClause where_clause;

      SmallString<COLUMN_NAME_MAX_LENGTH> primary_join_column_name;
      SmallString<COLUMN_NAME_MAX_LENGTH> secondary_join_column_name;
      if (join_type != SqlJoinType::None) {
        // parse join columns

        if (!this->has_input()) {
          error.set_unexpected_end();
          return;
        }
        if (*this->peek() ==
            tokenizer::SqlToken(tokenizer::SqlKeyword::WHERE)) {
          // consume where
          this->read();
        }

        // read table name
        SmallString<TABLE_NAME_MAX_LENGTH> first_table_name;
        this->read_table_name(first_table_name, error);
        if (!error.is_ok())
          return;

        // TODO: validate .
        this->read();

        // read column name
        SmallString<COLUMN_NAME_MAX_LENGTH> first_column_name;
        this->read_column_name(first_column_name, error);
        if (!error.is_ok())
          return;

        if (first_table_name == table_name_alias) {
          primary_join_column_name = first_column_name;
        } else if (first_table_name == joined_table_name_alias) {
          panic("TODO: update joined table name");
        } else {
          panic("Unknown table name while parsing");
        }

        // TODO: validate =
        this->read();

        // read table name
        SmallString<TABLE_NAME_MAX_LENGTH> second_table_name;
        this->read_table_name(second_table_name, error);
        if (!error.is_ok())
          return;

        // TODO: validate .
        this->read();

        // read column name
        SmallString<COLUMN_NAME_MAX_LENGTH> second_column_name;
        this->read_column_name(second_column_name, error);
        if (!error.is_ok())
          return;

        if (second_table_name == table_name_alias) {
          primary_join_column_name = second_column_name;
        } else if (second_table_name == joined_table_name_alias) {
          secondary_join_column_name = second_column_name;
        } else {
          panic("Unknown table name while parsing");
        }
      } else {
        // parse where clause
        if (!this->has_input()) {
          error.set_unexpected_end();
          return;
        }
        if (*this->peek() ==
            tokenizer::SqlToken(tokenizer::SqlKeyword::WHERE)) {
          this->read_where_clause(where_clause, error);
          if (!error.is_ok())
            return;
          has_where_clause = true;
        }
      }

      // read ;
      this->read_semicolon(error);
      if (!error.is_ok())
        return;

      SqlStatementSelect select{table_name,
                                column_names,
                                has_where_clause,
                                where_clause,
                                join_type,
                                joined_table_name,
                                primary_join_column_name,
                                secondary_join_column_name};
      statements.push_back(SqlStatement(select));
      break;
    }
    case tokenizer::SqlKeyword::ALTER: {
      // consume token
      this->read();

      {
        // ALTER TABLE <table> ADD a3 float;
        const tokenizer::SqlKeyword *keyword = nullptr;
        this->read_keyword(&keyword, error);
        if (!error.is_ok())
          return;
        // TODO: Return error
        assert(*keyword == tokenizer::SqlKeyword::TABLE);
      }

      // read table name
      SmallString<TABLE_NAME_MAX_LENGTH> table_name;
      this->read_table_name(table_name, error);
      if (!error.is_ok())
        return;

      {
        const tokenizer::SqlKeyword *keyword = nullptr;
        this->read_keyword(&keyword, error);
        if (!error.is_ok())
          return;
        // TODO: Return error
        assert(*keyword == tokenizer::SqlKeyword::ADD);
      }

      // read column name
      SmallString<COLUMN_NAME_MAX_LENGTH> column_name;
      this->read_column_name(column_name, error);
      if (!error.is_ok())
        return;

      // read sql type
      SqlType type;
      this->read_type(&type, error);
      if (!error.is_ok())
        return;

      // read ;
      this->read_semicolon(error);
      if (!error.is_ok())
        return;

      SqlStatementAlter alter{
        table_name,
        column : SqlColumn{
          name : column_name,
          type,
        },
      };
      statements.push_back(SqlStatement(alter));

      break;
    }
    case tokenizer::SqlKeyword::INSERT: {
      // consume token
      this->read();

      // insert into <table> values(1,	'Gizmo',      	19.99);

      {
        const tokenizer::SqlKeyword *keyword = nullptr;
        this->read_keyword(&keyword, error);
        if (!error.is_ok())
          return;
        // TODO: Return error
        assert(*keyword == tokenizer::SqlKeyword::INTO);
      }

      // read table name
      SmallString<TABLE_NAME_MAX_LENGTH> table_name;
      this->read_table_name(table_name, error);
      if (!error.is_ok())
        return;

      {
        const tokenizer::SqlKeyword *keyword = nullptr;
        this->read_keyword(&keyword, error);
        if (!error.is_ok())
          return;
        // TODO: Return error
        assert(*keyword == tokenizer::SqlKeyword::VALUES);
      }

      // read (
      this->read_left_parenthesis(error);
      if (!error.is_ok())
        return;

      // read sql value
      SqlValue first_value;
      this->read_sql_value(first_value, error);
      if (!error.is_ok())
        return;

      SmallVec<COLUMN_MAX, SqlValue> values;
      values.push(first_value);

      const tokenizer::SqlToken *token = this->peek();
      while (token && *token == tokenizer::SqlToken::comma()) {
        // read comma
        this->read();

        // read sql value
        SqlValue value;
        this->read_sql_value(value, error);
        if (!error.is_ok())
          return;

        if (!values.push(value)) {
          error.set_limit_reached();
          return;
        }

        token = this->peek();
      }

      // read )
      this->read_right_parenthesis(error);
      if (!error.is_ok())
        return;

      // read ;
      this->read_semicolon(error);
      if (!error.is_ok())
        return;

      SqlStatementInsert insert{table_name, values};
      statements.push_back(SqlStatement(insert));

      break;
    }
    case tokenizer::SqlKeyword::UPDATE: {
      // consume token
      this->read();
      // update Product set name = 'Gizmo' where name = 'SuperGizmo';

      // read table name
      SmallString<TABLE_NAME_MAX_LENGTH> table_name;
      this->read_table_name(table_name, error);
      if (!error.is_ok())
        return;

      // read "set"
      {
        const tokenizer::SqlKeyword *keyword = nullptr;
        this->read_keyword(&keyword, error);
        if (!error.is_ok())
          return;
        // TODO: Return error
        assert(*keyword == tokenizer::SqlKeyword::SET);
      }

      // read column name
      const tokenizer::SqlIdentifier *column_identifier = nullptr;
      this->read_identifier(&column_identifier, error);
      if (!error.is_ok())
        return;
      if (column_identifier->value.size() > COLUMN_NAME_MAX_LENGTH) {
        error.set_limit_reached();
        return;
      }

      // read '='
      {
        const tokenizer::SqlOperator *op = nullptr;
        this->read_operator(&op, error);
        if (!error.is_ok())
          return;
        // TODO: Return error
        assert(*op == tokenizer::SqlOperator::Equals);
      }

      // read value
      SqlValue value;
      this->read_sql_value(value, error);
      if (!error.is_ok())
        return;

      /// read where clause
      SqlWhereClause where_clause;
      this->read_where_clause(where_clause, error);
      if (!error.is_ok())
        return;

      /// read ;
      this->read_semicolon(error);
      if (!error.is_ok())
        return;

      SmallString<COLUMN_NAME_MAX_LENGTH> column_name(
          column_identifier->value.get_ptr(), column_identifier->value.size());

      SqlStatementUpdate insert{table_name, column_name, value, where_clause};
      statements.push_back(SqlStatement(insert));
      break;
    }
    case tokenizer::SqlKeyword::DELETE: {
      // consume token
      this->read();

      // delete from product where name = 'Gizmo';

      // read from
      {
        const tokenizer::SqlKeyword *keyword = nullptr;
        this->read_keyword(&keyword, error);
        if (!error.is_ok())
          return;
        // TODO: Return error
        assert(*keyword == tokenizer::SqlKeyword::FROM);
      }

      // read table name
      SmallString<TABLE_NAME_MAX_LENGTH> table_name;
      this->read_table_name(table_name, error);
      if (!error.is_ok())
        return;

      // read where clause
      SqlWhereClause where_clause;
      this->read_where_clause(where_clause, error);

      // read ;
      this->read_semicolon(error);
      if (!error.is_ok())
        return;

      SqlStatementDelete insert{table_name, where_clause};
      statements.push_back(SqlStatement(insert));

      break;
    }
    case tokenizer::SqlKeyword::BEGIN: {
      // consume token
      this->read();

      // BEGIN TRANSACTION;

      // TODO: Verify token is TRANSACTION
      this->read();

      // read ;
      this->read_semicolon(error);
      if (!error.is_ok())
        return;

      SqlStatementBeginTransaction begin_transaction;
      statements.push_back(SqlStatement(begin_transaction));

      break;
    }
    case tokenizer::SqlKeyword::COMMIT: {
      // consume token
      this->read();

      // COMMIT;

      // read ;
      this->read_semicolon(error);
      if (!error.is_ok())
        return;

      SqlStatementCommitTransaction commit_transaction;
      statements.push_back(SqlStatement(commit_transaction));

      break;
    }
    default:
      // TODO: return err
      std::cout << "UNEXPECTED KEYWORD TOKEN: " << *token << std::endl;
      abort();
    }
    break;
  default:
    // TODO: return err
    std::cout << "UNEXPECTED TOKEN: " << *token << std::endl;
    abort();
  };
}

/// Check if there is input remaining
bool SqlParser::has_input() { return this->position < this->tokens.size(); }
// TODO: Dynamically tokenize here instead of doing it upfront
/// Peek the next token, return nullptr if there is none.
const tokenizer::SqlToken *SqlParser::peek() {
  if (!this->has_input())
    return nullptr;
  return &this->tokens[this->position];
}
/// Read the next token, marking it as read.
const tokenizer::SqlToken *SqlParser::read() {
  const tokenizer::SqlToken *token = this->peek();
  if (token != nullptr)
    position += 1;
  return token;
}
/// Read the next keyword, marking it as read.
void SqlParser::read_keyword(const tokenizer::SqlKeyword **keyword,
                             SqlParserError &error) {
  const tokenizer::SqlToken *token = this->read();
  if (token == nullptr) {
    error.set_unexpected_end();
    return;
  }

  tokenizer::SqlTokenType token_type = token->token_type();
  if (token_type != tokenizer::SqlTokenType::KEYWORD) {
    error.set_unexpected_token(token_type);
    return;
  }

  *keyword = &token->keyword();
}
/// Read the next identifier, marking it as read.
void SqlParser::read_identifier(const tokenizer::SqlIdentifier **identifier,
                                SqlParserError &error) {
  const tokenizer::SqlToken *token = this->read();
  if (token == nullptr) {
    error.set_unexpected_end();
    return;
  }

  tokenizer::SqlTokenType token_type = token->token_type();
  if (token_type != tokenizer::SqlTokenType::IDENTIFIER) {
    error.set_unexpected_token(token_type);
    return;
  }

  *identifier = &token->identifier();
}
/// Read the next keyword, marking it as read.
void SqlParser::read_semicolon(SqlParserError &error) {
  const tokenizer::SqlToken *token = this->read();
  if (token == nullptr) {
    error.set_unexpected_end();
    return;
  }

  tokenizer::SqlTokenType token_type = token->token_type();
  if (token_type != tokenizer::SqlTokenType::SEMICOLON) {
    error.set_unexpected_token(token_type);
    return;
  }
}
/// Read the next keyword, marking it as read.
void SqlParser::read_left_parenthesis(SqlParserError &error) {
  const tokenizer::SqlToken *token = this->read();
  if (token == nullptr) {
    error.set_unexpected_end();
    return;
  }
  tokenizer::SqlTokenType token_type = token->token_type();
  if (token_type != tokenizer::SqlTokenType::LEFT_PARENTHESIS) {
    error.set_unexpected_token(token_type);
    return;
  }
}
/// Read the next keyword, marking it as read.
void SqlParser::read_right_parenthesis(SqlParserError &error) {
  const tokenizer::SqlToken *token = this->read();
  if (token == nullptr) {
    error.set_unexpected_end();
    return;
  }
  tokenizer::SqlTokenType token_type = token->token_type();
  if (token_type != tokenizer::SqlTokenType::RIGHT_PARENTHESIS) {
    error.set_unexpected_token(token_type);
    return;
  }
}
/// Read the next integer, marking it as read.
void SqlParser::read_integer_literal(
    const tokenizer::SqlIntegerLiteral **integer_literal,
    SqlParserError &error) {
  const tokenizer::SqlToken *token = this->read();
  if (token == nullptr) {
    error.set_unexpected_end();
    return;
  }
  tokenizer::SqlTokenType token_type = token->token_type();
  if (token_type != tokenizer::SqlTokenType::INTEGER_LITERAL) {
    error.set_unexpected_token(token_type);
    return;
  }

  *integer_literal = &token->integer_literal();
}
/// Read a table name.
///
/// Converts to lower case.
void SqlParser::read_table_name(SmallString<TABLE_NAME_MAX_LENGTH> &table_name,
                                SqlParserError &error) {
  const tokenizer::SqlIdentifier *table_identifier = nullptr;
  this->read_identifier(&table_identifier, error);
  if (!error.is_ok())
    return;
  if (table_identifier->value.size() > TABLE_NAME_MAX_LENGTH) {
    error.set_limit_reached();
    return;
  }

  // already did length check above, copy and lower-case
  for (size_t i = 0; i < table_identifier->value.size(); i++) {
    table_name.push(tolower(table_identifier->value[i]));
  }
}

} // namespace parser
} // namespace basic_sql