/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#include "SqlValue.h"

namespace basic_sql {
/// fmt sql value
std::ostream &operator<<(std::ostream &os, const SqlValue &v) {
  SqlValueType v_type = v.type();
  switch (v_type) {
  case SqlValueType::Null:
    os << "";
    break;
  case SqlValueType::Integer:
    os << v.get_integer();
    break;
  case SqlValueType::String:
    os << v.get_string();
    break;
  case SqlValueType::Float:
    os << v.get_float();
    break;
  default:
    std::string message(
        "unknown SqlValueType in `std::ostream &operator<<(std::ostream &os, "
        "const SqlValue &v)`: ");
    message += std::to_string((int)v_type);
    panic(message.c_str());
    break;
  }

  return os;
}

/// cmp sql values
bool operator==(const SqlValue &lhs, const SqlValue &rhs) {
  SqlValueType lhs_type = lhs.type();
  SqlValueType rhs_type = rhs.type();
  if (lhs_type != rhs_type) {
    // float-int comparison
    if (lhs_type == SqlValueType::Integer && rhs_type == SqlValueType::Float) {
      return lhs.get_integer() == rhs.get_float();
    }
    if (lhs_type == SqlValueType::Float && rhs_type == SqlValueType::Integer) {
      return lhs.get_float() == rhs.get_integer();
    }

    return false;
  }

  switch (lhs_type) {
  case SqlValueType::String:
    return lhs.get_string() == rhs.get_string();
  case SqlValueType::Integer:
    return lhs.get_integer() == rhs.get_integer();
  default:
    panic("Unknown SqlValueType in `bool operator==(const SqlValue &lhs, const "
          "SqlValue &rhs)`");
    break;
  }

  return false;
}

/// gt sql values
bool operator>(const SqlValue &lhs, const SqlValue &rhs) {
  SqlValueType lhs_type = lhs.type();
  SqlValueType rhs_type = rhs.type();
  if (lhs_type != rhs_type) {
    // float-int comparison
    if (lhs_type == SqlValueType::Integer && rhs_type == SqlValueType::Float) {
      return lhs.get_integer() > rhs.get_float();
    }
    if (lhs_type == SqlValueType::Float && rhs_type == SqlValueType::Integer) {
      return lhs.get_float() > rhs.get_integer();
    }

    return false;
  }

  switch (lhs_type) {
  case SqlValueType::Float:
    return lhs.get_float() > rhs.get_float();
  default:
    panic(
        "Unknown `SqlValueType` in `bool operator==(const SqlValue &lhs, const "
        "SqlValue &rhs)`");
    return false;
  }

  return false;
}

/// ne sql values
bool operator!=(const SqlValue &lhs, const SqlValue &rhs) {
  return !(lhs == rhs);
}
} // namespace basic_sql