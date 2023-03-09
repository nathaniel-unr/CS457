#include "SqlToken.h"
#include <cassert>

namespace basic_sql {
namespace tokenizer {
/// fmt a sql keyword
std::ostream &operator<<(std::ostream &os, const SqlKeyword &t) {
  switch (t) {
  case SqlKeyword::CREATE:
    os << "CREATE";
    break;
  case SqlKeyword::DATABASE:
    os << "DATABASE";
    break;
  case SqlKeyword::DROP:
    os << "DROP";
    break;
  case SqlKeyword::USE:
    os << "USE";
    break;
  case SqlKeyword::TABLE:
    os << "TABLE";
    break;
  case SqlKeyword::SELECT:
    os << "SELECT";
    break;
  case SqlKeyword::FROM:
    os << "FROM";
    break;
  case SqlKeyword::ALTER:
    os << "ALTER";
    break;
  case SqlKeyword::ADD:
    os << "ADD";
    break;
  case SqlKeyword::INSERT:
    os << "INSERT";
    break;
  case SqlKeyword::INTO:
    os << "INTO";
    break;
  case SqlKeyword::VALUES:
    os << "VALUES";
    break;
  case SqlKeyword::UPDATE:
    os << "UPDATE";
    break;
  case SqlKeyword::SET:
    os << "SET";
    break;
  case SqlKeyword::WHERE:
    os << "WHERE";
    break;
  case SqlKeyword::DELETE:
    os << "DELETE";
    break;
  case SqlKeyword::INNER:
    os << "INNER";
    break;
  case SqlKeyword::JOIN:
    os << "JOIN";
    break;
  case SqlKeyword::LEFT:
    os << "LEFT";
    break;
  case SqlKeyword::OUTER:
    os << "OUTER";
    break;
  case SqlKeyword::ON:
    os << "ON";
    break;
  case SqlKeyword::BEGIN:
    os << "BEGIN";
    break;
  case SqlKeyword::TRANSACTION:
    os << "TRANSACTION";
    break;
  case SqlKeyword::COMMIT:
    os << "COMMIT";
    break;
  default:
    panic("unknown SqlKeyword in ostream fmt");
    break;
  }

  return os;
}

/// fmt a sql token type
std::ostream &operator<<(std::ostream &os, const SqlTokenType &t) {
  switch (t) {
  case SqlTokenType::KEYWORD:
    os << "KEYWORD";
    break;
  case SqlTokenType::IDENTIFIER:
    os << "IDENTIFIER";
    break;
  case SqlTokenType::SEMICOLON:
    os << "SEMICOLON";
    break;
  case SqlTokenType::LEFT_PARENTHESIS:
    os << "LEFT_PARENTHESIS";
    break;
  case SqlTokenType::RIGHT_PARENTHESIS:
    os << "RIGHT_PARENTHESIS";
    break;
  case SqlTokenType::COMMA:
    os << "COMMA";
    break;
  case SqlTokenType::INTEGER_LITERAL:
    os << "INTEGER_LITERAL";
    break;
  case SqlTokenType::TYPE:
    os << "TYPE";
    break;
  case SqlTokenType::ASTERISK:
    os << "ASTERISK";
    break;
  case SqlTokenType::STRING_LITERAL:
    os << "STRING_LITERAL";
    break;
  case SqlTokenType::FLOAT_LITERAL:
    os << "FLOAT_LITERAL";
    break;
  case SqlTokenType::OPERATOR:
    os << "OPERATOR";
    break;
  default:
    panic("unknown SqlTokenType in ostream fmt");
    return os;
  }

  return os;
}

/// cmp sql identifiers
bool operator==(const SqlIdentifier &lhs, const SqlIdentifier &rhs) {
  return lhs.value == rhs.value;
}
/// fmt a sql identifier to a stream
std::ostream &operator<<(std::ostream &os, const SqlIdentifier &t) {
  return (os << t.value);
}

/// cmp sql integer literals
bool operator==(const SqlIntegerLiteral &lhs, const SqlIntegerLiteral &rhs) {
  return lhs.value == rhs.value;
}
/// fmt a sql integer literal to a stream
std::ostream &operator<<(std::ostream &os, const SqlIntegerLiteral &t) {
  return (os << t.value);
}

/// cmp sql string literals
bool operator==(const SqlStringLiteral &lhs, const SqlStringLiteral &rhs) {
  return lhs.value == rhs.value;
}
/// fmt a sql string literal to a stream
std::ostream &operator<<(std::ostream &os, const SqlStringLiteral &t) {
  return (os << t.value);
}

/// cmp sql float literals
bool operator==(const SqlFloatLiteral &lhs, const SqlFloatLiteral &rhs) {
  return lhs.value == rhs.value;
}
/// fmt a sql float literal to a stream
std::ostream &operator<<(std::ostream &os, const SqlFloatLiteral &t) {
  return (os << t.value);
}

/// Make a new sql token from a keyword
SqlToken::SqlToken(SqlKeyword keyword)
    : m_token_type(SqlTokenType::KEYWORD), m_keyword(keyword) {}
/// Make a new sql token from an ident
SqlToken::SqlToken(SqlIdentifier identifier)
    : m_token_type(SqlTokenType::IDENTIFIER), m_identifier(identifier) {}
/// Make a new sql token from an int literal
SqlToken::SqlToken(SqlIntegerLiteral integer_literal)
    : m_token_type(SqlTokenType::INTEGER_LITERAL),
      m_integer_literal(integer_literal) {}
/// Make a new sql token from a sql type
SqlToken::SqlToken(SqlType type)
    : m_token_type(SqlTokenType::TYPE), m_type(type) {}
/// Make a new sql token from a string literal
SqlToken::SqlToken(SqlStringLiteral string_literal)
    : m_token_type(SqlTokenType::STRING_LITERAL),
      m_string_literal(string_literal) {}
/// Make a new sql token from a float literal
SqlToken::SqlToken(SqlFloatLiteral float_literal)
    : m_token_type(SqlTokenType::FLOAT_LITERAL),
      m_float_literal(float_literal) {}
/// Make a new sql token from a sql operator
SqlToken::SqlToken(SqlOperator op)
    : m_token_type(SqlTokenType::OPERATOR), m_operator(op) {}
/// Make a new sql token from a semicolon
SqlToken SqlToken::semicolon() { return SqlToken(SqlTokenType::SEMICOLON); }
/// Make a new sql token from a left parenthesis
SqlToken SqlToken::left_parenthesis() {
  return SqlToken(SqlTokenType::LEFT_PARENTHESIS);
}
/// Make a new sql token from a right parenthesis
SqlToken SqlToken::right_parenthesis() {
  return SqlToken(SqlTokenType::RIGHT_PARENTHESIS);
}
/// Make a new sql token from a comma
SqlToken SqlToken::comma() { return SqlToken(SqlTokenType::COMMA); }
/// Make a new sql token from an asterisk
SqlToken SqlToken::asterisk() { return SqlToken(SqlTokenType::ASTERISK); }
/// Make a new sql token from a period
SqlToken SqlToken::period() { return SqlToken(SqlTokenType::PERIOD); }
/// Returns true if this token is a keyword.
bool SqlToken::is_keyword() const {
  return this->m_token_type == SqlTokenType::KEYWORD;
}
/// Returns true if this token is an identifier.
bool SqlToken::is_identifier() const {
  return this->m_token_type == SqlTokenType::IDENTIFIER;
}
/// Returns true if this token is an integer literal.
bool SqlToken::is_integer_literal() const {
  return this->m_token_type == SqlTokenType::INTEGER_LITERAL;
}
/// Returns true if this token is a type.
bool SqlToken::is_type() const {
  return this->m_token_type == SqlTokenType::TYPE;
}
/// Returns true if this token is a string literal.
bool SqlToken::is_string_literal() const {
  return this->m_token_type == SqlTokenType::STRING_LITERAL;
}
/// Returns true if this token is a float literal.
bool SqlToken::is_float_literal() const {
  return this->m_token_type == SqlTokenType::FLOAT_LITERAL;
}
/// Returns true if this token is an operator.
bool SqlToken::is_operator() const {
  return this->m_token_type == SqlTokenType::OPERATOR;
}
/// Get the keyword
///
/// This token must be a keyword.
const SqlKeyword &SqlToken::keyword() const {
  assert(this->is_keyword());
  return this->m_keyword;
}
/// Get the identifier
///
/// This token must be an identifier.
const SqlIdentifier &SqlToken::identifier() const {
  assert(this->is_identifier());
  return this->m_identifier;
}
/// Get the integer literal
///
/// This token must be an integer literal.
const SqlIntegerLiteral &SqlToken::integer_literal() const {
  assert(this->is_integer_literal());
  return this->m_integer_literal;
}
/// Get the string literal
///
/// This token must be a string literal.
const SqlStringLiteral &SqlToken::string_literal() const {
  assert(this->is_string_literal());
  return this->m_string_literal;
}
/// Get the float literal
///
/// This token must be a float literal.
const SqlFloatLiteral &SqlToken::float_literal() const {
  assert(this->is_float_literal());
  return this->m_float_literal;
}
/// Get the operator
///
/// This token must be an operator.
const SqlOperator &SqlToken::op() const {
  assert(this->is_operator());
  return this->m_operator;
}
/// Get the type
///
/// This token must be a type.
const SqlType &SqlToken::type() const {
  assert(this->is_type());
  return this->m_type;
}
/// Get the type of the token.
SqlTokenType SqlToken::token_type() const { return this->m_token_type; }
/// Private sql token constructor for internal use
SqlToken::SqlToken(SqlTokenType type) : m_token_type(type) {}
/// fmt sql token
std::ostream &operator<<(std::ostream &os, const SqlToken &t) {
  switch (t.token_type()) {
  case SqlTokenType::KEYWORD:
    os << SqlTokenType::KEYWORD << "(" << t.keyword() << ")";
    break;
  case SqlTokenType::IDENTIFIER:
    os << SqlTokenType::IDENTIFIER << "(" << t.identifier() << ")";
    break;
  case SqlTokenType::SEMICOLON:
    os << SqlTokenType::SEMICOLON;
    break;
  case SqlTokenType::LEFT_PARENTHESIS:
    os << SqlTokenType::LEFT_PARENTHESIS;
    break;
  case SqlTokenType::RIGHT_PARENTHESIS:
    os << SqlTokenType::RIGHT_PARENTHESIS;
    break;
  case SqlTokenType::COMMA:
    os << SqlTokenType::COMMA;
    break;
  case SqlTokenType::INTEGER_LITERAL:
    os << SqlTokenType::INTEGER_LITERAL << "(" << t.integer_literal() << ")";
    break;
  case SqlTokenType::TYPE:
    os << SqlTokenType::TYPE << "(" << t.type() << ")";
    break;
  case SqlTokenType::ASTERISK:
    os << SqlTokenType::ASTERISK;
    break;
  case SqlTokenType::STRING_LITERAL:
    os << SqlTokenType::STRING_LITERAL;
    break;
  case SqlTokenType::FLOAT_LITERAL:
    os << SqlTokenType::FLOAT_LITERAL;
    break;
  case SqlTokenType::OPERATOR:
    os << SqlTokenType::OPERATOR;
    break;
  default:
    os << t.token_type();
    panic("unknown SqlToken in fmt");
    break;
  }
  return os;
}
/// cmp sql token
bool operator==(const SqlToken &lhs, const SqlToken &rhs) {
  SqlTokenType lhs_type = lhs.token_type();
  if (lhs_type != rhs.token_type())
    return false;

  // We check above to see that they are the same type.
  // Then, we can safely match and compare their values.
  switch (lhs_type) {
  case SqlTokenType::KEYWORD:
    return lhs.keyword() == rhs.keyword();
  case SqlTokenType::IDENTIFIER:
    return lhs.identifier() == rhs.identifier();
  case SqlTokenType::SEMICOLON:
    return true;
  case SqlTokenType::LEFT_PARENTHESIS:
    return true;
  case SqlTokenType::RIGHT_PARENTHESIS:
    return true;
  case SqlTokenType::COMMA:
    return true;
  case SqlTokenType::INTEGER_LITERAL:
    return lhs.integer_literal() == rhs.integer_literal();
  case SqlTokenType::TYPE:
    return lhs.type() == rhs.type();
  case SqlTokenType::ASTERISK:
    return true;
  default:
    panic("unknown SqlToken in cmp");
    return false;
  }
}
/// cmp ne sql token
bool operator!=(const SqlToken &lhs, const SqlToken &rhs) {
  return !(lhs == rhs);
}
} // namespace tokenizer
} // namespace basic_sql