/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SQL_TOKEN_H_
#define _SQL_TOKEN_H_

#include "ConstStringSlice.h"
#include "Util.h"
#include "tokenizer/SqlType.h"
#include <ostream>

namespace basic_sql {
namespace tokenizer {

/// A sql keyword
enum class SqlKeyword {
  CREATE,
  DATABASE,
  DROP,
  USE,
  TABLE,
  SELECT,
  FROM,
  ALTER,
  ADD,
  INSERT,
  INTO,
  VALUES,
  UPDATE,
  SET,
  WHERE,
  DELETE,
  INNER,
  JOIN,
  LEFT,
  OUTER,
  ON,
  BEGIN,
  TRANSACTION,
  COMMIT,
};
/// fmt a sql keyword to a stream
std::ostream &operator<<(std::ostream &os, const SqlKeyword &t);

/// The sql token type
enum class SqlTokenType {
  KEYWORD,
  IDENTIFIER,
  SEMICOLON,
  LEFT_PARENTHESIS,
  RIGHT_PARENTHESIS,
  COMMA,
  INTEGER_LITERAL,
  TYPE,
  ASTERISK,
  STRING_LITERAL,
  FLOAT_LITERAL,
  OPERATOR,
  PERIOD,
};
/// fmt a sql token type to a stream
std::ostream &operator<<(std::ostream &os, const SqlTokenType &t);

/// A sql identifier
struct SqlIdentifier {
  ConstStringSlice value;
};
/// cmp sql idents
bool operator==(const SqlIdentifier &lhs, const SqlIdentifier &rhs);
/// fmt sql idents
std::ostream &operator<<(std::ostream &os, const SqlIdentifier &t);

/// A sql integer literal
struct SqlIntegerLiteral {
  /// The value
  int value;
};
/// cmp sql int literals
bool operator==(const SqlIntegerLiteral &lhs, const SqlIntegerLiteral &rhs);
/// fmt sql int literals
std::ostream &operator<<(std::ostream &os, const SqlIntegerLiteral &t);

/// A sql string literal
struct SqlStringLiteral {
  /// The value
  ConstStringSlice value;
};
/// cmp sql string literals
bool operator==(const SqlStringLiteral &lhs, const SqlStringLiteral &rhs);
/// fmt sql string literals
std::ostream &operator<<(std::ostream &os, const SqlStringLiteral &t);

/// A sql string literal
struct SqlFloatLiteral {
  /// The value
  float value;
};
/// cmp sql string literals
bool operator==(const SqlFloatLiteral &lhs, const SqlFloatLiteral &rhs);
/// fmt sql string literals
std::ostream &operator<<(std::ostream &os, const SqlFloatLiteral &t);

/// an operator
enum class SqlOperator { Equals, GreaterThan, NotEqual };

/// A sql token
class SqlToken {
public:
  /// Make a new sql token from a keyword
  SqlToken(SqlKeyword keyword);
  /// Make a new sql token from an ident
  SqlToken(SqlIdentifier identifier);
  /// Make a new sql token from an int literal
  SqlToken(SqlIntegerLiteral integer_literal);
  /// Make a new sql token from a sql type
  SqlToken(SqlType type);
  /// Make a new sql token from a string literal
  SqlToken(SqlStringLiteral string_literal);
  /// Make a new sql token from a float literal
  SqlToken(SqlFloatLiteral float_literal);
  /// Make a new sql token from an operator
  SqlToken(SqlOperator op);
  /// Make a new sql token from a semicolon
  static SqlToken semicolon();
  /// Make a new sql token from a left parenthesis
  static SqlToken left_parenthesis();
  /// Make a new sql token from a right parenthesis
  static SqlToken right_parenthesis();
  /// Make a new sql token from a comma
  static SqlToken comma();
  /// Make a new sql token from an asterisk
  static SqlToken asterisk();
  /// Make a new sql token from a period
  static SqlToken period();

  /// Returns true if this token is a keyword.
  bool is_keyword() const;

  /// Returns true if this token is an identifier.
  bool is_identifier() const;

  /// Returns true if this token is an integer literal.
  bool is_integer_literal() const;

  /// Returns true if this token is a type.
  bool is_type() const;

  /// Returns true if this token is a string literal.
  bool is_string_literal() const;

  /// Returns true if this token is a float literal.
  bool is_float_literal() const;

  /// Returns true if this token is an operator.
  bool is_operator() const;

  /// Get the keyword
  ///
  /// This token must be a keyword.
  const SqlKeyword &keyword() const;

  /// Get the identifier
  ///
  /// This token must be an identifier.
  const SqlIdentifier &identifier() const;

  /// Get the integer literal
  ///
  /// This token must be an integer literal.
  const SqlIntegerLiteral &integer_literal() const;

  /// Get the type
  ///
  /// This token must be a type.
  const SqlType &type() const;

  /// Get the string literal
  ///
  /// This token must be a string literal.
  const SqlStringLiteral &string_literal() const;

  /// Get the float literal
  ///
  /// This token must be a string literal.
  const SqlFloatLiteral &float_literal() const;

  /// Get the operator
  ///
  /// This token must be an operator
  const SqlOperator &op() const;

  /// Get the type of the token.
  SqlTokenType token_type() const;

private:
  /// Private sql token constructor for internal use
  SqlToken(SqlTokenType type);

  SqlTokenType m_token_type;
  union {
    SqlKeyword m_keyword;
    SqlIdentifier m_identifier;
    SqlIntegerLiteral m_integer_literal;
    SqlType m_type;
    SqlStringLiteral m_string_literal;
    SqlFloatLiteral m_float_literal;
    SqlOperator m_operator;
  };
};
/// fmt a sql token
std::ostream &operator<<(std::ostream &os, const SqlToken &t);
/// cmp a sql token
bool operator==(const SqlToken &lhs, const SqlToken &rhs);
/// cmp ne a sql token
bool operator!=(const SqlToken &lhs, const SqlToken &rhs);
} // namespace tokenizer
} // namespace basic_sql
#endif