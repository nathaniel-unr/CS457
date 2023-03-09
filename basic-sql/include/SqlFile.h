/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SQL_FILE_
#define _SQL_FILE_

#include "SqlError.h"
#include <cassert>
#include <string>

namespace basic_sql {
/// An interface to a file
class SqlFile {
public:
  /// Initializes the file with a name.
  ///
  /// This does not open the file.
  SqlFile() = delete;
  SqlFile(std::string name);
  SqlFile(SqlFile &other) = delete;
  SqlFile &operator=(const SqlFile &other) = delete;
  SqlFile(SqlFile &&other) noexcept;
  SqlFile &operator=(SqlFile &&other);
  /// Try close file, ignore error.
  ~SqlFile();

  /// Open the file.
  void open(const char *flags, SqlError &error);

  /// Return `true` if this file is closed.
  bool is_closed() const;

  /// Close a file.
  void close(SqlError &error);

  /// Write bytes to a file
  void write(const uint8_t *ptr, size_t len, SqlError &error);

  /// Read bytes from a file to a ptr
  void read(uint8_t *ptr, size_t len, SqlError &error);

  /// Seek the file. Uses absolute positioning.
  void seek(long int offset, SqlError &error);

  /// Get the current postion
  void position(size_t &position, SqlError &error);

  /// Get the name of the file
  const std::string &name() const;

  /// Flush the file
  void flush(SqlError &error) {
    // TODO: Return error
    assert(fflush(this->m_file) == 0);
  }

  /// Close and remove this file.
  void remove_file(SqlError &error) {
    this->close(error);
    if (!error.is_ok())
      return;
    int remove_code = remove(this->name().c_str());
    assert(remove_code == 0);
  }

  /// write a byte n times
  void write_byte_n(uint8_t byte, size_t n, SqlError &error) {
    for (size_t i = 0; i < n; i++) {
      this->write(&byte, 1, error);
      if (!error.is_ok())
        return;
    }
  }

private:
  FILE *m_file;
  std::string m_name;
};

} // namespace basic_sql
#endif