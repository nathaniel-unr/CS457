/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#include "SqlTokenizer.h"
#include <math.h>

namespace basic_sql {
namespace tokenizer {
/// Make a new tokeniser over a string
SqlTokenizer::SqlTokenizer(const std::string &input)
    : input(input), position(0) {}
/// Tokenize
void SqlTokenizer::tokenize(std::vector<SqlToken> &tokens,
                            SqlTokenizerError &error) {
  const char *start_char = this->peek();
  // Exit if end on input
  while (start_char) {
    if (isalpha(*start_char)) {
      this->read();

      const char *c = this->peek();
      size_t len = 1;
      while (c && (isalnum(*c) || *c == '_')) {
        this->read();
        c = this->peek();
        len += 1;
      }

      ConstStringSlice slice(start_char, len);

      // Check for keywords and types
      if (slice.case_insensitive_compare("CREATE")) {
        tokens.push_back(SqlToken(SqlKeyword::CREATE));
      } else if (slice.case_insensitive_compare("DATABASE")) {
        tokens.push_back(SqlToken(SqlKeyword::DATABASE));
      } else if (slice.case_insensitive_compare("DROP")) {
        tokens.push_back(SqlToken(SqlKeyword::DROP));
      } else if (slice.case_insensitive_compare("USE")) {
        tokens.push_back(SqlToken(SqlKeyword::USE));
      } else if (slice.case_insensitive_compare("TABLE")) {
        tokens.push_back(SqlToken(SqlKeyword::TABLE));
      } else if (slice.case_insensitive_compare("SELECT")) {
        tokens.push_back(SqlToken(SqlKeyword::SELECT));
      } else if (slice.case_insensitive_compare("FROM")) {
        tokens.push_back(SqlToken(SqlKeyword::FROM));
      } else if (slice.case_insensitive_compare("ALTER")) {
        tokens.push_back(SqlToken(SqlKeyword::ALTER));
      } else if (slice.case_insensitive_compare("ADD")) {
        tokens.push_back(SqlToken(SqlKeyword::ADD));
      } else if (slice.case_insensitive_compare("INSERT")) {
        tokens.push_back(SqlToken(SqlKeyword::INSERT));
      } else if (slice.case_insensitive_compare("INTO")) {
        tokens.push_back(SqlToken(SqlKeyword::INTO));
      } else if (slice.case_insensitive_compare("VALUES")) {
        tokens.push_back(SqlToken(SqlKeyword::VALUES));
      } else if (slice.case_insensitive_compare("UPDATE")) {
        tokens.push_back(SqlToken(SqlKeyword::UPDATE));
      } else if (slice.case_insensitive_compare("SET")) {
        tokens.push_back(SqlToken(SqlKeyword::SET));
      } else if (slice.case_insensitive_compare("WHERE")) {
        tokens.push_back(SqlToken(SqlKeyword::WHERE));
      } else if (slice.case_insensitive_compare("DELETE")) {
        tokens.push_back(SqlToken(SqlKeyword::DELETE));
      } else if (slice.case_insensitive_compare("INNER")) {
        tokens.push_back(SqlToken(SqlKeyword::INNER));
      } else if (slice.case_insensitive_compare("JOIN")) {
        tokens.push_back(SqlToken(SqlKeyword::JOIN));
      } else if (slice.case_insensitive_compare("LEFT")) {
        tokens.push_back(SqlToken(SqlKeyword::LEFT));
      } else if (slice.case_insensitive_compare("OUTER")) {
        tokens.push_back(SqlToken(SqlKeyword::OUTER));
      } else if (slice.case_insensitive_compare("ON")) {
        tokens.push_back(SqlToken(SqlKeyword::ON));
      } else if (slice.case_insensitive_compare("BEGIN")) {
        tokens.push_back(SqlToken(SqlKeyword::BEGIN));
      } else if (slice.case_insensitive_compare("TRANSACTION")) {
        tokens.push_back(SqlToken(SqlKeyword::TRANSACTION));
      } else if (slice.case_insensitive_compare("COMMIT")) {
        tokens.push_back(SqlToken(SqlKeyword::COMMIT));
      } else if (slice.case_insensitive_compare("INT")) {
        tokens.push_back(SqlToken(SqlType::INT));
      } else if (slice.case_insensitive_compare("VARCHAR")) {
        tokens.push_back(SqlToken(SqlType::VARCHAR));
      } else if (slice.case_insensitive_compare("FLOAT")) {
        tokens.push_back(SqlToken(SqlType::FLOAT));
      } else if (slice.case_insensitive_compare("CHAR")) {
        tokens.push_back(SqlToken(SqlType::CHAR));
      } else {
        tokens.push_back(SqlToken(SqlIdentifier{slice}));
      }
    } else if (isdigit(*start_char)) {
      // tokenize int literal or float literal

      this->read();
      const char *c = this->peek();
      // already read 1 char
      size_t len = 1;
      while (c && isdigit(*c)) {
        this->read();
        c = this->peek();
        len += 1;
      }

      ConstStringSlice slice(start_char, len);
      // TODO: Better limit tests
      size_t slice_size = slice.size();
      size_t slice_size_limit = 6;
      if (slice_size > slice_size_limit) {
        error.last_char = start_char;
        error.position = this->position;
        error.message.clear();
        error.message.append("integer literal too large");
        error.kind = SqlTokenizerErrorKind::Other;
        return;
      }

      int value = 0;
      for (size_t i = 0; i < slice_size; i++) {
        value = (value * 10) + (slice[i] - '0');
      }

      if (c && *c == '.') {
        // tokenize float literal

        // consume .
        this->read();

        // setup next loop
        c = this->peek();
        start_char = c;
        len = 0;
        while (c && isdigit(*c)) {
          this->read();
          c = this->peek();
          len += 1;
        }

        // TODO: Error
        assert(len != 0);

        ConstStringSlice slice(start_char, len);
        // TODO: Better limit tests
        size_t slice_size = slice.size();
        size_t slice_size_limit = 6;
        if (slice_size > slice_size_limit) {
          error.last_char = start_char;
          error.position = this->position;
          error.message.clear();
          error.message.append("float literal too large");
          error.kind = SqlTokenizerErrorKind::Other;
          return;
        }

        int fraction_value = 0;
        for (size_t i = 0; i < slice_size; i++) {
          fraction_value = (fraction_value * 10) + (slice[i] - '0');
        }

        float fraction_value_double =
            ((float)fraction_value) / pow(10.0f, (float)slice_size);

        float float_value = ((float)value) + fraction_value_double;

        tokens.push_back(SqlToken(SqlFloatLiteral{value : float_value}));
      } else {
        // tokenize int literal
        tokens.push_back(SqlToken(SqlIntegerLiteral{value}));
      }
    } else if (*start_char == '\'') {
      // tokenize string literal

      this->read();
      const char *c = this->peek();
      // already read 1 char
      size_t len = 1;
      while (c && *c != '\'') {
        this->read();
        c = this->peek();
        len += 1;
      }

      if (c == nullptr) {
        // TODO: return error, unexpected eof
        panic("unexpected eof while tokenizing string");
      }
      // consume '
      this->read();

      // TODO: return error
      assert(len > 1);

      ConstStringSlice value(start_char + 1, len - 1);
      tokens.push_back(SqlToken(SqlStringLiteral{value}));
    } else if (*start_char == ' ') {
      this->read();
    } else if (*start_char == ';') {
      this->read();
      tokens.push_back(SqlToken::semicolon());
    } else if (*start_char == '(') {
      this->read();
      tokens.push_back(SqlToken::left_parenthesis());
    } else if (*start_char == ')') {
      this->read();
      tokens.push_back(SqlToken::right_parenthesis());
    } else if (*start_char == ',') {
      this->read();
      tokens.push_back(SqlToken::comma());
    } else if (*start_char == '*') {
      this->read();
      tokens.push_back(SqlToken::asterisk());
    } else if (*start_char == '=') {
      this->read();
      tokens.push_back(SqlToken(SqlOperator::Equals));
    } else if (*start_char == '>') {
      this->read();
      tokens.push_back(SqlToken(SqlOperator::GreaterThan));
    } else if (*start_char == '!') {
      this->read();

      const char *c = this->read();
      if (c == nullptr) {
        panic("c is null");
      }

      if (*c != '=') {
        panic("expected =");
      }

      tokens.push_back(SqlToken(SqlOperator::NotEqual));
    } else if (*start_char == '.') {
      this->read();
      tokens.push_back(SqlToken::period());
    } else if (isspace(*start_char)) {
      // TODO: Parse whitespace?
      this->read();
    } else {
      error.last_char = start_char;
      error.position = this->position;
      error.message.clear();
      error.message.append("unexpected char type '");
      error.message.push(*start_char);
      error.message.push('\'');
      error.kind = SqlTokenizerErrorKind::UnexpectedChar;
      return;
    }

    start_char = this->peek();
  }
}

/// Returns the next char without consuming it.
///
/// If the string is completly consumed, it returns a nullptr.
const char *SqlTokenizer::peek() {
  if (this->is_finished())
    return nullptr;
  return &this->input.at(this->position);
}
/// Reads the next char, consuming it.
///
/// If the string is completly consumed, it returns a nullptr.
const char *SqlTokenizer::read() {
  const char *c = this->peek();
  if (c != nullptr)
    this->position += 1;
  return c;
}
} // namespace tokenizer
} // namespace basic_sql