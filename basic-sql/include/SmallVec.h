/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SMALL_VEC_H_
#define _SMALL_VEC_H_

#include <cassert>
#include <cstddef>

namespace basic_sql {
/// A vec allocated on the stack
template <size_t N, typename T> class SmallVec {
public:
  /// Make an empty SmallVec
  SmallVec() : m_len(0) {}
  /// Push a value.
  ///
  /// Returns false if it cannot be pushed.
  bool push(T data) {
    if (m_len >= N)
      return false;
    this->m_data[m_len] = data;
    this->m_len++;
    return true;
  }

  /// Get the size.
  size_t size() const { return m_len; }

  /// Index the slice.
  ///
  /// i must not be greater than the length.
  const T &operator[](size_t i) const {
    assert(i < this->m_len);
    return this->m_data[i];
  }
  T &operator[](size_t i) {
    assert(i < this->m_len);
    return this->m_data[i];
  }

private:
  T m_data[N];
  size_t m_len;
};
} // namespace basic_sql
#endif