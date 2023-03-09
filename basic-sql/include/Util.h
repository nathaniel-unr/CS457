/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _UTIL_H_
#define _UTIL_H_

#include "SmallString.h"
#include "SqlFile.h"
#include "parser/SqlType.h"
#include "tokenizer/SqlType.h"
#include <iostream>

namespace basic_sql {
/// Abort with a message
void panic(const char *string);
} // namespace basic_sql
#endif