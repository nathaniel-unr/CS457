/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SQL_TOKENIZER_ERROR_H_
#define _SQL_TOKENIZER_ERROR_H_

#include "SmallString.h"

namespace basic_sql {
namespace tokenizer {
/// The max error length of the message in a sql tokenizer error
const size_t MAX_TOKENIZER_ERROR_LENGTH = 1024;

/// The SQL tokenizer error kind
enum class SqlTokenizerErrorKind {
  /// No issues
  Ok = 0,
  /// An unexpected char was encountered
  UnexpectedChar = 1,
  /// An unspecified error occured
  Other = 2,
};

/// The error that is produced when input cannot be tokenized
class SqlTokenizerError {
public:
  /// Make a new error that is ok.
  SqlTokenizerError()
      : last_char(nullptr), position(0), message("Ok"),
        kind(SqlTokenizerErrorKind::Ok) {}

  /// The last examined char. May be null. References the input.
  const char *last_char;
  /// The position in the input
  size_t position;
  /// The error message
  SmallString<MAX_TOKENIZER_ERROR_LENGTH> message;
  /// The error kind
  SqlTokenizerErrorKind kind;

  /// Check if this is OK
  bool is_ok() { return this->kind == SqlTokenizerErrorKind::Ok; }
};
} // namespace tokenizer
} // namespace basic_sql
#endif