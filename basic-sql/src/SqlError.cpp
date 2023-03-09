#include "SqlError.h"

namespace basic_sql {
/// fmt a sql error type
std::ostream &operator<<(std::ostream &os, const SqlErrorType &t) {
  switch (t) {
  case SqlErrorType::Ok:
    os << "Ok";
    break;
  case SqlErrorType::BadFileOpen:
    os << "BadFileOpen";
    break;
  case SqlErrorType::BadFileClose:
    os << "BadFileClose";
    break;
  case SqlErrorType::FileAlreadyOpened:
    os << "FileAlreadyOpened";
    break;
  case SqlErrorType::FileClosed:
    os << "FileClosed";
    break;
  case SqlErrorType::LimitReached:
    os << "LimitReached";
    break;
  case SqlErrorType::DirStatError:
    os << "DirStatError";
    break;
  case SqlErrorType::BadMkDir:
    os << "BadMkDir";
    break;
  case SqlErrorType::AlreadyExists:
    os << "AlreadyExists";
    break;
  case SqlErrorType::Missing:
    os << "Missing";
    break;
  case SqlErrorType::Io:
    os << "Io";
    break;
  default:
    std::cout << "Unknown SqlErrorType (" << (int)t
              << ") in `std::ostream "
                 "&operator<<(std::ostream &os, const SqlErrorType &t)`"
              << std::endl;
    abort();
  }

  return os;
}
} // namespace basic_sql