/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SQL_TOKENIZER_H_
#define _SQL_TOKENIZER_H_
#include "ConstStringSlice.h"
#include "SmallString.h"
#include "SqlToken.h"
#include "SqlTokenizerError.h"
#include <ostream>
#include <vector>

namespace basic_sql {
namespace tokenizer {
/// A sql tokenizer
class SqlTokenizer {
public:
  /// Make a new tokeniser over a string
  SqlTokenizer(const std::string &input);

  /// Tokenize
  void tokenize(std::vector<SqlToken> &tokens, SqlTokenizerError &error);

protected:
private:
  /// The input
  const std::string &input;
  /// The position in the input.
  size_t position;

  /// Returns true if the entire string has been read.
  bool is_finished() { return position >= input.size(); }

  /// Returns the next char without consuming it.
  ///
  /// If the string is completly consumed, it returns a nullptr.
  const char *peek();

  /// Reads the next char, consuming it.
  ///
  /// If the string is completly consumed, it returns a nullptr.
  const char *read();
};
} // namespace tokenizer
} // namespace basic_sql
#endif