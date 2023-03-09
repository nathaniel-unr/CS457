/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SMALL_STRING_H_
#define _SMALL_STRING_H_

#include <cassert>
#include <cstring>
#include <ostream>

namespace basic_sql {
/// A dynamically sized string on the stack.
template <size_t N> class SmallString {
public:
  /// Make an empty SmallString
  SmallString() : m_len(0) {}
  /// Copy constructor
  SmallString(const SmallString<N> &other) {
    this->clear();
    this->append(other.get_ptr(), other.size());
  }
  SmallString &operator=(const SmallString<N> &other) {
    this->clear();
    this->append(other.get_ptr(), other.size());
    return *this;
  }
  /// Make a small string from a c string.
  ///
  /// May not copy the entire c string if it is too long.
  SmallString(const char *ptr) : SmallString(ptr, strlen(ptr)) {}
  /// Make a small string copying from ptr up to len or N, whichever is smaller
  SmallString(const char *ptr, size_t len) {
    this->clear();
    size_t append_len = N < len ? N : len;
    this->append(ptr, append_len);
  }

  /// Get a ptr to the data.
  const char *get_ptr() const { return this->m_data; }

  /// Get a ptr to the data.
  char *get_ptr() { return this->m_data; }

  /// Get the size
  size_t size() const { return this->m_len; }

  /// Append data from a c string.
  ///
  /// Returns false if the data was not written.
  bool append(const char *data) { return this->append(data, strlen(data)); }

  /// Append data from a ptr and len.
  ///
  /// Returns false if the data was not written.
  bool append(const char *data, size_t len) {
    // Length check
    if (this->m_len + len > N)
      return false;

    // Unsafe append
    for (size_t i = 0; i < len; i++) {
      this->m_data[this->m_len + i] = data[i];
    }
    this->m_len += len;

    assert(this->m_len <= N);

    return true;
  }

  /// Append a char.
  ///
  /// Returns false if the data was not written.
  bool push(char data) {
    if (1 + this->m_len > N)
      return false;
    this->m_data[this->m_len] = data;
    this->m_len++;

    assert(this->m_len <= N);

    return true;
  }

  /// Clear the string.
  void clear() { this->m_len = 0; }

  /// Forcibly sets the size
  void unsafe_set_size(size_t len) { this->m_len = len; }

  /// Index the slice.
  ///
  /// i must not be greater than the length.
  char operator[](size_t i) const {
    assert(i < this->m_len);
    assert(m_len < N);

    return this->m_data[i];
  }

protected:
private:
  char m_data[N];
  size_t m_len;
};

/// ostream formatter
template <size_t N>
std::ostream &operator<<(std::ostream &os, const SmallString<N> &s) {
  assert(s.size() <= N);

  for (size_t i = 0; i < s.size(); i++)
    os << s[i];

  return os;
}

/// Compare
template <size_t N>
bool operator==(const SmallString<N> &lhs, const SmallString<N> &rhs) {
  size_t lhs_size = lhs.size();
  if (lhs_size != rhs.size())
    return false;

  for (size_t i = 0; i < lhs_size; i++)
    if (lhs[i] != rhs[i])
      return false;

  return true;
}
} // namespace basic_sql
#endif