#include "SqlFile.h"

namespace basic_sql {
SqlFile::SqlFile(std::string name) : m_file(nullptr), m_name(name) {}
SqlFile::SqlFile(SqlFile &&other) noexcept
    : m_file(other.m_file), m_name(other.m_name) {
  other.m_file = nullptr;
}
SqlFile &SqlFile::operator=(SqlFile &&other) {
  SqlError error;
  this->close(error);

  // Moved
  this->m_file = other.m_file;
  other.m_file = nullptr;

  // Copied, not moved
  this->m_name = other.m_name;

  return *this;
}
/// Try close file, ignore error.
SqlFile::~SqlFile() {
  SqlError error;
  this->close(error);
}
/// Open the file.
void SqlFile::open(const char *flags, SqlError &error) {
  if (this->m_file != nullptr) {
    error.set_file_already_opened();
    return;
  }

  FILE *file = fopen(this->m_name.c_str(), flags);
  if (file == nullptr) {
    error.set_bad_file_open();
    return;
  }

  this->m_file = file;
}
/// Return `true` if this file is closed.
bool SqlFile::is_closed() const { return this->m_file == nullptr; }
/// Close a file.
void SqlFile::close(SqlError &error) {
  if (!this->is_closed()) {
    int code = fclose(this->m_file);
    this->m_file = nullptr;

    // Returns 0 on success, EOF on failure.
    if (code != 0)
      error.set_bad_file_close();
  }
}
void SqlFile::write(const uint8_t *ptr, size_t len, SqlError &error) {
  // check if closed
  if (this->is_closed()) {
    error.set_file_closed();
    return;
  }

  // write
  size_t written = fwrite(ptr, 1, len, this->m_file);
  if (written != len) {
    error.set_io();
    return;
  }
}
/// Read bytes from a file to a ptr
void SqlFile::read(uint8_t *ptr, size_t len, SqlError &error) {
  // check if closed
  if (this->is_closed()) {
    error.set_file_closed();
    return;
  }

  // read
  size_t read = fread(ptr, 1, len, this->m_file);
  if (read != len) {
    error.set_io();
    return;
  }
}
/// Seek the file. Uses absolute positioning.
void SqlFile::seek(long int offset, SqlError &error) {
  // check if closed
  if (this->is_closed()) {
    error.set_file_closed();
    return;
  }

  // seek
  int error_code = fseek(this->m_file, offset, SEEK_SET);
  if (error_code != 0) {
    error.set_io();
    return;
  }
}
/// Get the current postion
void SqlFile::position(size_t &position, SqlError &error) {
  if (this->is_closed()) {
    error.set_file_closed();
    return;
  }

  long int cur_pos = ftell(this->m_file);
  if (cur_pos == -1) {
    error.set_io();
    return;
  }
  position = cur_pos;
}
/// Get the name of the file
const std::string &SqlFile::name() const { return m_name; }
} // namespace basic_sql