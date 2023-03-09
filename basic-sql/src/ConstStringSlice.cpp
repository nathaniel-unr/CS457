/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#include "ConstStringSlice.h"

#include <cassert>
#include <cstring>

namespace basic_sql {
/// Make an empty ConstStringSlice.
ConstStringSlice::ConstStringSlice() : ptr(nullptr), len(0) {}
/// Make a ConstStringSlice from a c string.
ConstStringSlice::ConstStringSlice(const char *c_str)
    : ptr(c_str), len(strlen(c_str)) {}
/// Make a ConstStringSlice reference from parts.
ConstStringSlice::ConstStringSlice(const char *ptr, size_t len)
    : ptr(ptr), len(len) {}
/// Returns true if this is empty.
bool ConstStringSlice::is_empty() const { return this->len == 0; }
/// Return the number of chars.
size_t ConstStringSlice::size() const { return this->len; }
/// Get a ptr to the data.
const char *ConstStringSlice::get_ptr() const { return this->ptr; }

// TODO: Accept slice instead
// TODO: rename to equals_case_insensitive?
/// Compare this slice with a c string, ignoring case.
/// Returns true if they are equal.
bool ConstStringSlice::case_insensitive_compare(const char *other) const {
  size_t other_len = strlen(other);
  if (other_len != this->len)
    return false;
  for (size_t i = 0; i < this->len; i++)
    if (other[i] == 0 || (std::toupper((*this)[i]) != std::toupper(other[i])))
      return false;

  return true;
}

/// See if this slice starts with another
bool ConstStringSlice::starts_with(ConstStringSlice slice) {
  if (slice.size() > this->size())
    return false;
  for (size_t i = 0; i < slice.size(); i++) {
    if (this->ptr[i] != slice[i])
      return false;
  }

  return true;
}

/// Index the slice.
///
/// i must not be greater than the length.
const char &ConstStringSlice::operator[](size_t i) const {
  assert(i < this->len);
  return this->ptr[i];
}

/// fmt slice to stream
std::ostream &operator<<(std::ostream &os, const ConstStringSlice &t) {
  for (size_t i = 0; i < t.size(); i++)
    os << t[i];

  return os;
}

// cmp 2 slices
bool operator==(const ConstStringSlice &lhs, const ConstStringSlice &rhs) {
  size_t lhs_size = lhs.size();
  if (lhs_size != rhs.size())
    return false;

  for (size_t i = 0; i < lhs_size; i++)
    if (lhs[i] != rhs[i])
      return false;

  return true;
}
} // namespace basic_sql