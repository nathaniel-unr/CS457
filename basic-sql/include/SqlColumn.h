/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SQL_COLUMN_H_
#define _SQL_COLUMN_H_

#include "parser/SqlType.h"

namespace basic_sql {
namespace parser {

/// A SQL column
struct SqlColumn {
  /// The column name
  SmallString<COLUMN_NAME_MAX_LENGTH> name;

  /// The column type
  SqlType type;
};

} // namespace parser
} // namespace basic_sql
#endif