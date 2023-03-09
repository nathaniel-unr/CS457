/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SQL_VALUE_H_
#define _SQL_VALUE_H_

#include "ConstStringSlice.h"
#include "Limits.h"
#include "Util.h"

namespace basic_sql {
/// A sql value type
enum class SqlValueType {
  Null,
  Integer,
  Float,
  String,
};

/// A Sql Value
class SqlValue {
public:
  /// Make a new sql value
  SqlValue() : m_type(SqlValueType::Null) {}
  SqlValue(const SqlValue &other) {
    switch (other.m_type) {
    case SqlValueType::Integer:
      this->m_integer = other.m_integer;
      break;
    case SqlValueType::String:
      this->m_string = other.m_string;
      break;
    case SqlValueType::Float:
      this->m_float = other.m_float;
      break;
    case SqlValueType::Null:
      break;
    default:
      std::string message("unknown `SqlValue` type in copy constructor: ");
      message += std::to_string((int)other.m_type);
      panic(message.c_str());
    }
    this->m_type = other.m_type;
  }
  SqlValue operator=(const SqlValue &other) {
    switch (other.m_type) {
    case SqlValueType::Integer:
      this->m_integer = other.m_integer;
      break;
    case SqlValueType::String:
      this->m_string = other.m_string;
      break;
    case SqlValueType::Float:
      this->m_float = other.m_float;
      break;
    case SqlValueType::Null:
      break;
    default:
      std::string message("unknown `SqlValue` type in assign constructor: ");
      message += std::to_string((int)other.m_type);
      panic(message.c_str());
      break;
    }
    this->m_type = other.m_type;

    return *this;
  }

  /// Set the value of this to be a string.
  ///
  /// Return false on failure.
  bool set_string(const ConstStringSlice &slice) {
    return this->set_string(slice.get_ptr(), slice.size());
  }
  bool set_string(const char *ptr, size_t len) {
    if (len > MAX_TYPE_SIZE)
      return false;
    this->m_type = SqlValueType::String;
    this->m_string = SmallString<MAX_TYPE_SIZE>(ptr, len);
    return true;
  }

  /// Set the value of this to a float
  void set_float(float value) {
    this->m_type = SqlValueType::Float;
    this->m_float = value;
  }

  /// Set the value of this to an integer
  void set_integer(uint32_t value) {
    this->m_type = SqlValueType::Integer;
    this->m_integer = value;
  }

  /// get the type
  SqlValueType type() const { return this->m_type; }

  /// Get the value as a float
  ///
  /// This must be a float
  const float &get_float() const { return this->m_float; }

  /// Get the value as a string
  ///
  /// This must be a string
  const SmallString<MAX_TYPE_SIZE> &get_string() const {
    return this->m_string;
  }
  /// Get the value as a integer
  ///
  /// This must be an integer
  const uint32_t &get_integer() const { return this->m_integer; }

private:
  SqlValueType m_type;
  union {
    uint32_t m_integer;
    float m_float;
    SmallString<MAX_TYPE_SIZE> m_string;
  };
};

/// fmt sql value
std::ostream &operator<<(std::ostream &os, const SqlValue &v);
/// cmp sql values
bool operator==(const SqlValue &lhs, const SqlValue &rhs);
/// gt sql values
bool operator>(const SqlValue &lhs, const SqlValue &rhs);
/// ne sql values
bool operator!=(const SqlValue &lhs, const SqlValue &rhs);
} // namespace basic_sql
#endif