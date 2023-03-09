/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _LIMITS_H_
#define _LIMITS_H_

#include <cstddef>

namespace basic_sql {
/// The max database name size
const size_t DATABASE_MAX_NAME_SIZE = 16;
/// The max size of a column name
const size_t COLUMN_NAME_MAX_LENGTH = 16;
/// The max size of a table name
const size_t TABLE_NAME_MAX_LENGTH = 16;
/// The max # of columns in a table
const size_t COLUMN_MAX = 16;
/// The max type size
const size_t MAX_TYPE_SIZE = 64;
} // namespace basic_sql

#endif