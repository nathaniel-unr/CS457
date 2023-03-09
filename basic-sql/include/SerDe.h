/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SER_DE_H_
#define _SER_DE_H_

#include "SmallString.h"
#include "SqlError.h"
#include "SqlFile.h"
#include "SqlValue.h"
#include "parser/SqlType.h"
#include "tokenizer/SqlType.h"

namespace basic_sql {

/// Write a small string to a file.
template <size_t N>
void write_small_string_to_file(SmallString<N> small_string, SqlFile &file,
                                SqlError &error) {
  // Write len
  uint8_t len = small_string.size();
  file.write(&len, 1, error);
  if (!error.is_ok())
    return;

  // pad for storage
  while (small_string.push(0)) {
  }

  // Write body
  file.write((const uint8_t *)small_string.get_ptr(), small_string.size(),
             error);
  if (!error.is_ok())
    return;
}

/// Read a small string from a file
template <size_t N>
void read_small_string_from_file(SmallString<N> &small_string, SqlFile &file,
                                 SqlError &error) {
  // read len
  uint8_t len = 0;
  file.read(&len, 1, error);
  if (!error.is_ok())
    return;
  small_string.clear();

  // TODO: maybe error here. This should only be possible with data corruption.
  assert(len <= N);

  // read data
  // always read N to update the position pointer correctly.
  // we already have the buffer for it too.
  file.read((uint8_t *)small_string.get_ptr(), N, error);
  if (!error.is_ok())
    return;
  small_string.unsafe_set_size(len);
}

/// write a tokenizer sql type
void write_tokenizer_sql_type(tokenizer::SqlType type, SqlFile &file,
                              SqlError &error);

/// read tokenizer type
void read_sql_tokenizer_type(tokenizer::SqlType &type, SqlFile &file,
                             SqlError &error);

/// read a sql type
void read_sql_type(parser::SqlType &type, SqlFile &file, SqlError &error);

/// write a sql type
void write_sql_type(const parser::SqlType &type, SqlFile &file,
                    SqlError &error);

/// write a sql value to a file
void write_sql_value(const SqlValue &value, SqlFile &file, SqlError &error);

/// read a sql value from a file
void read_sql_value(SqlValue &value, tokenizer::SqlType expected_type,
                    SqlFile &file, SqlError &error);

} // namespace basic_sql
#endif