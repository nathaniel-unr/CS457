/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#include "SerDe.h"

namespace basic_sql {
/// write a tokenizer sql type
void write_tokenizer_sql_type(tokenizer::SqlType type, SqlFile &file,
                              SqlError &error) {
  // transalte to type id
  uint8_t type_id = 0;
  switch (type) {
  case tokenizer::SqlType::INT:
    type_id = 1;
    break;
  case tokenizer::SqlType::VARCHAR:
    type_id = 2;
    break;
  case tokenizer::SqlType::FLOAT:
    type_id = 3;
    break;
  case tokenizer::SqlType::CHAR:
    type_id = 4;
    break;
  default:
    panic("unknown `tokenizer::SqlType` in `write_tokenizer_sql_type`");
  }

  // write type id
  file.write(&type_id, 1, error);
  if (!error.is_ok())
    return;
}

/// read tokenizer type
void read_sql_tokenizer_type(tokenizer::SqlType &type, SqlFile &file,
                             SqlError &error) {
  // read type id
  uint8_t type_id = 0;
  file.read(&type_id, 1, error);
  if (!error.is_ok())
    return;

  // translate to type
  switch (type_id) {
  case 1:
    type = tokenizer::SqlType::INT;
    break;
  case 2:
    type = tokenizer::SqlType::VARCHAR;
    break;
  case 3:
    type = tokenizer::SqlType::FLOAT;
    break;
  case 4:
    type = tokenizer::SqlType::CHAR;
    break;
  default:
    std::string panic_message = "unknown `basic_sql::tokenizer::SqlType` in "
                                "`read_sql_tokenizer_type`, type_id (";
    panic_message += std::to_string((int)type_id);
    panic_message += ')';
    panic(panic_message.c_str());
  }
}

/// read a sql type
void read_sql_type(parser::SqlType &type, SqlFile &file, SqlError &error) {
  // read type size
  uint8_t type_size = 0;
  file.read(&type_size, 1, error);
  if (!error.is_ok())
    return;

  // read type
  tokenizer::SqlType tokenizer_type = tokenizer::SqlType::INT;
  read_sql_tokenizer_type(tokenizer_type, file, error);
  if (!error.is_ok())
    return;

  type.size = type_size;
  type.type = tokenizer_type;
}

/// write a sql type
void write_sql_type(const parser::SqlType &type, SqlFile &file,
                    SqlError &error) {
  // Write type size
  uint8_t type_size = type.size;
  file.write(&type_size, 1, error);
  if (!error.is_ok())
    return;

  // write type
  write_tokenizer_sql_type(type.type, file, error);
  if (!error.is_ok())
    return;
}

/// write a sql value to a file
void write_sql_value(const SqlValue &value, SqlFile &file, SqlError &error) {
  SqlValueType value_type = value.type();

  size_t written = 0;
  switch (value_type) {
  case SqlValueType::Float: {
    // TODO: endian
    file.write((const uint8_t *)&value.get_float(), sizeof(float), error);
    if (!error.is_ok())
      return;
    written = sizeof(float);
    break;
  }
  case SqlValueType::String: {
    const SmallString<MAX_TYPE_SIZE> &string_data = value.get_string();
    size_t string_data_size = string_data.size();

    // write size
    uint8_t size = string_data_size;
    file.write((const uint8_t *)&size, 1, error);
    if (!error.is_ok())
      return;

    // write string
    file.write((const uint8_t *)string_data.get_ptr(), string_data_size, error);
    if (!error.is_ok())
      return;
    written = string_data_size + 1;

    break;
  }
  case SqlValueType::Integer: {
    // TODO: endian
    // size of int on file is 4
    file.write((const uint8_t *)&value.get_integer(), 4, error);
    if (!error.is_ok())
      return;
    written = 4;
    break;
  }
  default:
    panic("unknown `SqlValue` in `write_sql_value`");
  }

  // TODO: I really should reserve MAX_TYPE_SIZE + 1 since string needs 1 extra
  // for length storage
  //
  // pad empty data
  file.write_byte_n(0, MAX_TYPE_SIZE - written, error);
  if (!error.is_ok())
    return;
}

/// read a sql value from a file
void read_sql_value(SqlValue &sql_value, tokenizer::SqlType expected_type,
                    SqlFile &file, SqlError &error) {
  size_t read = 0;
  switch (expected_type) {
  case tokenizer::SqlType::FLOAT: {
    // read float
    float value = 0.0;
    file.read((uint8_t *)&value, 4, error);
    if (!error.is_ok())
      return;

    sql_value.set_float(value);

    read = 4;
    break;
  }
  case tokenizer::SqlType::VARCHAR:
  case tokenizer::SqlType::CHAR: {
    // read string size
    uint8_t size = 0;
    file.read((uint8_t *)&size, 1, error);
    if (!error.is_ok())
      return;

    // read string body
    SmallString<MAX_TYPE_SIZE> value;
    file.read((uint8_t *)value.get_ptr(), size, error);
    if (!error.is_ok())
      return;
    value.unsafe_set_size(size);

    // TODO: do in place, avoid copy
    sql_value.set_string(value.get_ptr(), value.size());

    read = 1 + size;
    break;
  }
  case tokenizer::SqlType::INT: {
    // read int
    int value = 0;
    file.read((uint8_t *)&value, 4, error);
    if (!error.is_ok())
      return;

    sql_value.set_integer(value);

    read = 4;
    break;
  }
  default:
    panic("unknown type in `basic_sql::read_sql_value`");
    break;
  }

  // read empty data
  for (size_t i = read; i < MAX_TYPE_SIZE; i++) {
    uint8_t temp = 0;
    file.read(&temp, 1, error);
    if (!error.is_ok())
      return;
  }
}

} // namespace basic_sql