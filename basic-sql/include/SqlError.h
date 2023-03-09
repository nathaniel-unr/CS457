/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SQL_ERROR_H_
#define _SQL_ERROR_H_

#include <iostream>

namespace basic_sql {
/// A sql error type
enum class SqlErrorType {
  // No error
  Ok,
  /// Failed to open a file
  BadFileOpen,
  /// Failed to close a file
  BadFileClose,
  /// The file is already open
  FileAlreadyOpened,
  /// The file is closed
  FileClosed,
  /// A limit was reached
  LimitReached,
  /// Failed to stat a dir
  DirStatError,
  /// Failed to make a dir
  BadMkDir,
  /// Something already exists
  AlreadyExists,
  /// Something is missing
  Missing,
  /// Io Error
  Io,
  /// Invalid File
  InvalidFile,
};

/// fmt a sql error type
std::ostream &operator<<(std::ostream &os, const SqlErrorType &t);

/// A SQL Error
class SqlError {
public:
  /// Make a new sql error that is ok
  SqlError() : m_type(SqlErrorType::Ok) {}
  /// Returns true if there is no error.
  bool is_ok() { return this->m_type == SqlErrorType::Ok; }
  /// Get the error type
  SqlErrorType type() { return m_type; }
  /// Set the error to a file open error
  void set_bad_file_open() { this->m_type = SqlErrorType::BadFileOpen; }
  /// Set the error to a file close error
  void set_bad_file_close() { this->m_type = SqlErrorType::BadFileClose; }
  /// Set the error to file already opened
  void set_file_already_opened() {
    this->m_type = SqlErrorType::FileAlreadyOpened;
  }
  /// Set the error to file closed
  void set_file_closed() { this->m_type = SqlErrorType::FileClosed; }
  /// Set limit reached error
  void set_limit_reached() { this->m_type = SqlErrorType::LimitReached; }
  /// Set dir stat error
  void set_dir_stat() { this->m_type = SqlErrorType::DirStatError; }
  /// Set mkdir error
  void set_bad_mkdir() { this->m_type = SqlErrorType::BadMkDir; }
  /// Set already exists error
  void set_already_exists() { this->m_type = SqlErrorType::AlreadyExists; }
  /// Set missing error
  void set_missing() { this->m_type = SqlErrorType::Missing; }
  /// Set io error
  void set_io() { this->m_type = SqlErrorType::Io; }
  /// Set invalid file error
  void set_invalid_file() { this->m_type = SqlErrorType::InvalidFile; }

private:
  SqlErrorType m_type;
  union {
    int m_dir_stat;
  };
};
} // namespace basic_sql
#endif