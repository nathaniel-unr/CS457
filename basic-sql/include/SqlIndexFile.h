/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef SQL_INDEX_FILE
#define SQL_INDEX_FILE

#include "Limits.h"
#include "SerDe.h"
#include "SqlFile.h"
#include "Util.h"

namespace basic_sql {
/// index file magic #
static const char *SQL_INDEX_FILE_MAGIC = "index-db";
/// index file magic # length
static const size_t SQL_INDEX_FILE_MAGIC_SIZE = 8;

/// An index of tables
class SqlIndexFile {
public:
  /// Make a new sql file.
  SqlIndexFile(std::string name);
  SqlIndexFile(const SqlIndexFile &other) = delete;
  SqlIndexFile &operator=(SqlIndexFile &other) = delete;
  SqlIndexFile(SqlIndexFile &&other) noexcept;
  SqlIndexFile &operator=(SqlIndexFile &&other);

  /// Open the index file
  void open(bool create, SqlError &error);

  /// Seek to a table name index.
  void seek_to_table_name_index(size_t index, SqlError &error);

  /// Add the # of tables in the index.
  uint8_t get_num_tables();

  /// Get the index of a table name.
  ///
  /// Returns the index, or -1 if not found.
  /// If an error occured, the result is also -1;
  int index_of_table_name(const SmallString<TABLE_NAME_MAX_LENGTH> &table_name,
                          SqlError &error);

  /// Get the table name at the index.
  void get_table_name(size_t index,
                      SmallString<TABLE_NAME_MAX_LENGTH> &table_name,
                      SqlError &error);

  /// Insert the table name at the given index, overwriting it.
  void insert_table_name(size_t index,
                         const SmallString<TABLE_NAME_MAX_LENGTH> &table_name,
                         SqlError &error);

  /// Remove the table at the index.
  void remove(size_t i, SqlError &error);

  /// Update num_tables
  void update_num_tables(uint8_t new_num_tables, SqlError &error);

  /// Returns true if this is closed.
  bool is_closed() const;

  /// Close this file
  void close(SqlError &error);

  /// Get the file name
  const std::string &file_name() const;

  /// remove the table file and close this file.
  void remove_file(SqlError &error);

private:
  SqlFile m_file;
  uint8_t num_tables;
};
} // namespace basic_sql

#endif