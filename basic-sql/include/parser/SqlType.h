/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _BASIC_SQL_PARSER_SQL_TYPE_H_
#define _BASIC_SQL_PARSER_SQL_TYPE_H_

#include "tokenizer/SqlType.h"

namespace basic_sql {
namespace parser {
/// A SQL type
struct SqlType {
  /// The core type
  tokenizer::SqlType type;

  // Should be set to 1 if not variable.
  size_t size;
};
} // namespace parser
} // namespace basic_sql

#endif