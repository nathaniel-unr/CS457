/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SQL_PARSER_H_
#define _SQL_PARSER_H_

#include "SmallVec.h"
#include "SqlStatement.h"
#include "SqlTokenizer.h"
#include "SqlValue.h"

namespace basic_sql {
namespace parser {
/// The type of parser error
enum class SqlParserErrorType {
  /// Ok
  Ok = 0,
  /// Tokenizer error
  Tokenizer = 1,
  /// Unexpected end of input
  UnexpectedEnd = 2,
  /// Unexpected Token
  UnexpectedToken = 3,
  /// A Limit was reached
  LimitReached = 4,
};

/// A parser error
class SqlParserError {
public:
  /// Make a parser error that is ok.
  SqlParserError();
  /// Set this to a tokenizer error
  void set_tokenizer_error(tokenizer::SqlTokenizerError tokenizer_error);
  /// Set the error to be an unexpected end.
  void set_unexpected_end();
  /// Set the error to be an unexpected token.
  void set_unexpected_token(tokenizer::SqlTokenType token_type);
  /// Set the error to be limit reached.
  void set_limit_reached();
  /// Check if this error is ok.
  bool is_ok() const;
  /// Get the type
  SqlParserErrorType type() { return m_error_type; }
  const tokenizer::SqlTokenizerError &get_tokenizer_error() {
    return m_tokenizer_error;
  }

protected:
private:
  SqlParserErrorType m_error_type;
  union {
    tokenizer::SqlTokenizerError m_tokenizer_error;
    tokenizer::SqlTokenType m_unexpected_token_type;
  };
};

class SqlParser {
public:
  // TODO: maybe just proivde the token buffer, and a utility func to tokenize
  // the string and feed it
  // TODO: Consider parser reuse
  /// Make a new parser from the given input
  SqlParser(const std::string &input);
  /// Parse all statements into a vec.
  void parse_all(std::vector<SqlStatement> &statements, SqlParserError &error);
  /// Parse a single statement
  void parse_statement(std::vector<SqlStatement> &statements,
                       SqlParserError &error);

protected:
private:
  tokenizer::SqlTokenizer tokenizer;
  size_t position;
  std::vector<tokenizer::SqlToken> tokens;

  /// Check if there is input remaining.
  bool has_input();
  // TODO: Dynamically tokenize here instead of doing it upfront.
  /// Peek the next token, return nullptr if there is none.
  const tokenizer::SqlToken *peek();
  /// Read the next token, marking it as read.
  const tokenizer::SqlToken *read();
  /// Read the next keyword, marking it as read.
  void read_keyword(const tokenizer::SqlKeyword **keyword,
                    SqlParserError &error);
  /// Read the next identifier, marking it as read.
  void read_identifier(const tokenizer::SqlIdentifier **identifier,
                       SqlParserError &error);
  /// Read the next keyword, marking it as read.
  void read_semicolon(SqlParserError &error);
  /// Read the next keyword, marking it as read.
  void read_left_parenthesis(SqlParserError &error);
  /// Read the next keyword, marking it as read.
  void read_right_parenthesis(SqlParserError &error);
  /// Read the next integer literal, marking it as read.
  void read_integer_literal(
      const basic_sql::tokenizer::SqlIntegerLiteral **integer_literal,
      SqlParserError &error);

  /// Read the next type, marking it as read.
  void read_type(SqlType *type, SqlParserError &error) {
    const tokenizer::SqlToken *token = this->read();
    if (token == nullptr) {
      error.set_unexpected_end();
      return;
    }
    tokenizer::SqlTokenType token_type = token->token_type();
    if (token_type != tokenizer::SqlTokenType::TYPE) {
      error.set_unexpected_token(token_type);
      return;
    }

    size_t size = 1;
    if (token->type() == tokenizer::SqlType::VARCHAR ||
        token->type() == tokenizer::SqlType::CHAR) {
      this->read_left_parenthesis(error);
      if (!error.is_ok())
        return;
      const tokenizer::SqlIntegerLiteral *integer_literal = nullptr;
      this->read_integer_literal(&integer_literal, error);
      if (!error.is_ok())
        return;
      size = integer_literal->value;
      this->read_right_parenthesis(error);
      if (!error.is_ok())
        return;
    }

    *type = SqlType{
      type : token->type(),
      size,
    };
  }

  /// Read the next string literal, marking it as read.
  void read_string_literal(const tokenizer::SqlStringLiteral **string_literal,
                           SqlParserError &error) {
    const tokenizer::SqlToken *token = this->read();
    if (token == nullptr) {
      error.set_unexpected_end();
      return;
    }
    tokenizer::SqlTokenType token_type = token->token_type();
    if (token_type != tokenizer::SqlTokenType::STRING_LITERAL) {
      error.set_unexpected_token(token_type);
      return;
    }

    *string_literal = &token->string_literal();
  }

  /// Read the next float literal, marking it as read.
  void read_float_literal(const tokenizer::SqlFloatLiteral **float_literal,
                          SqlParserError &error) {
    const tokenizer::SqlToken *token = this->read();
    if (token == nullptr) {
      error.set_unexpected_end();
      return;
    }
    tokenizer::SqlTokenType token_type = token->token_type();
    if (token_type != tokenizer::SqlTokenType::FLOAT_LITERAL) {
      error.set_unexpected_token(token_type);
      return;
    }

    *float_literal = &token->float_literal();
  }

  /// Read a SqlValue
  void read_sql_value(SqlValue &value, SqlParserError &error) {
    const tokenizer::SqlToken *token = this->peek();
    if (token == nullptr) {
      error.set_unexpected_end();
      return;
    }
    tokenizer::SqlTokenType token_type = token->token_type();
    switch (token_type) {
    case tokenizer::SqlTokenType::STRING_LITERAL: {
      const tokenizer::SqlStringLiteral *literal = nullptr;
      this->read_string_literal(&literal, error);
      if (!error.is_ok())
        return;
      if (token == nullptr) {
        error.set_unexpected_end();
        return;
      }

      if (!value.set_string(literal->value)) {
        error.set_limit_reached();
        return;
      }

      break;
    }
    case tokenizer::SqlTokenType::FLOAT_LITERAL: {
      const tokenizer::SqlFloatLiteral *literal = nullptr;
      this->read_float_literal(&literal, error);
      if (!error.is_ok())
        return;
      if (token == nullptr) {
        error.set_unexpected_end();
        return;
      }
      value.set_float(literal->value);
      break;
    }
    case tokenizer::SqlTokenType::INTEGER_LITERAL: {
      const tokenizer::SqlIntegerLiteral *literal = nullptr;
      this->read_integer_literal(&literal, error);
      if (!error.is_ok())
        return;
      if (token == nullptr) {
        error.set_unexpected_end();
        return;
      }
      value.set_integer(literal->value);
      break;
    }
    default:
      error.set_unexpected_token(token_type);
      return;
    }
  }

  /// Read the next operator, marking it as read.
  void read_operator(const tokenizer::SqlOperator **op, SqlParserError &error) {
    const tokenizer::SqlToken *token = this->read();
    if (token == nullptr) {
      error.set_unexpected_end();
      return;
    }
    tokenizer::SqlTokenType token_type = token->token_type();
    if (token_type != tokenizer::SqlTokenType::OPERATOR) {
      error.set_unexpected_token(token_type);
      return;
    }

    *op = &token->op();
  }

  /// Read a where clause
  void read_where_clause(SqlWhereClause &clause, SqlParserError &error) {
    // read "where"
    {
      const tokenizer::SqlKeyword *keyword = nullptr;
      this->read_keyword(&keyword, error);
      if (!error.is_ok())
        return;
      // TODO: Return error
      assert(*keyword == tokenizer::SqlKeyword::WHERE);
    }

    // read where column name
    const tokenizer::SqlIdentifier *where_column_identifier = nullptr;
    this->read_identifier(&where_column_identifier, error);
    if (!error.is_ok())
      return;
    if (where_column_identifier->value.size() > COLUMN_NAME_MAX_LENGTH) {
      error.set_limit_reached();
      return;
    }

    // read op
    const tokenizer::SqlOperator *op = nullptr;
    this->read_operator(&op, error);
    if (!error.is_ok())
      return;

    // read value
    SqlValue value;
    this->read_sql_value(value, error);
    if (!error.is_ok())
      return;

    clause.column_name.clear();
    clause.column_name.append(where_column_identifier->value.get_ptr(),
                              where_column_identifier->value.size());
    clause.op = *op;
    clause.value = value;
  }

  /// Read a table name.
  ///
  /// Converts to lower case.
  void read_table_name(SmallString<TABLE_NAME_MAX_LENGTH> &table_name,
                       SqlParserError &error);

  /// Read a column name
  void read_column_name(SmallString<COLUMN_NAME_MAX_LENGTH> &column_name,
                        SqlParserError &error) {
    const tokenizer::SqlIdentifier *column_identifier = nullptr;
    this->read_identifier(&column_identifier, error);
    if (!error.is_ok())
      return;
    if (column_identifier->value.size() > COLUMN_NAME_MAX_LENGTH) {
      error.set_limit_reached();
      return;
    }

    // set return
    // length already validated
    // TODO: convert to lower case
    column_name.clear();
    column_name.append(column_identifier->value.get_ptr(),
                       column_identifier->value.size());
  }
};
} // namespace parser
} // namespace basic_sql
#endif