/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _CONST_STRING_SLICE_
#define _CONST_STRING_SLICE_

#include <cstddef>
#include <ostream>

namespace basic_sql {
/// A const view of a string
class ConstStringSlice {
public:
  /// Make an empty ConstStringSlice.
  ConstStringSlice();

  /// Make a ConstStringSlice from a c string.
  ConstStringSlice(const char *c_str);

  /// Make a ConstStringSlice reference from parts.
  ConstStringSlice(const char *ptr, size_t len);

  /// Returns true if this is empty.
  bool is_empty() const;

  /// Return the number of chars.
  size_t size() const;

  /// Get a ptr to the data.
  const char *get_ptr() const;

  // TODO: Accept slice instead
  // TODO: rename to equals_case_insensitive?
  /// Compare this slice with a c string, ignoring case.
  /// Returns true if they are equal.
  bool case_insensitive_compare(const char *other) const;

  /// See if this slice starts with another
  bool starts_with(ConstStringSlice slice);

  /// Trim whitespace from the end
  void trim_end() {
    while (this->len > 0 && isspace(this->ptr[len - 1]))
      this->len -= 1;
  }

  /// Trim whitespace from both ends
  void trim() {
    while (this->len > 0 && isspace(this->ptr[0]))
      this->ptr += 1;
    this->trim_end();
  }

  /// Index the slice.
  ///
  /// i must not be greater than the length.
  const char &operator[](size_t i) const;

protected:
private:
  const char *ptr;
  size_t len;
};

/// fmt slice to stream
std::ostream &operator<<(std::ostream &os, const ConstStringSlice &t);

// cmp 2 slices
bool operator==(const ConstStringSlice &lhs, const ConstStringSlice &rhs);

} // namespace basic_sql
#endif