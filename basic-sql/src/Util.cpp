/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#include "Util.h"

namespace basic_sql {
/// Abort with a message
void panic(const char *string) {
  std::cout << string << std::endl;
  abort();
}
} // namespace basic_sql