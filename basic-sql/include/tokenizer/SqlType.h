/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _BASIC_SQL_TOKENIZER_SQL_TOKEN_H_
#define _BASIC_SQL_TOKENIZER_SQL_TOKEN_H_

#include <ostream>

namespace basic_sql {
namespace tokenizer {
/// The sql type
enum class SqlType {
  INT,
  VARCHAR,
  FLOAT,
  CHAR,
};
/// fmt sql type
std::ostream &operator<<(std::ostream &os, const SqlType &t);
} // namespace tokenizer
} // namespace basic_sql
#endif