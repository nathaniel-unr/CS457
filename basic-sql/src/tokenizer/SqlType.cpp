#include "tokenizer/SqlType.h"
#include "Util.h"

namespace basic_sql {
namespace tokenizer {
/// fmt a sql type to a stream
std::ostream &operator<<(std::ostream &os, const SqlType &t) {
  switch (t) {
  case SqlType::INT:
    os << "INT";
    break;
  case SqlType::VARCHAR:
    os << "VARCHAR";
    break;
  case SqlType::FLOAT:
    os << "FLOAT";
    break;
  case SqlType::CHAR:
    os << "CHAR";
    break;
  default:
    panic("unknown `basic_sql::tokenizer::SqlType` in `std::ostream "
          "&operator<<(std::ostream &os, const SqlType &t)`");
  }

  return os;
}
} // namespace tokenizer
} // namespace basic_sql