/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#include "SqlIndexFile.h"

namespace basic_sql {
/// Make a new sql file.
SqlIndexFile::SqlIndexFile(std::string name) : m_file(name), num_tables(0) {}
SqlIndexFile::SqlIndexFile(SqlIndexFile &&other) noexcept
    : m_file(std::move(other.m_file)), num_tables(other.num_tables) {}
SqlIndexFile &SqlIndexFile::operator=(SqlIndexFile &&other) {
  SqlError error;
  this->close(error);
  this->m_file = std::move(other.m_file);
  this->num_tables = other.num_tables;
  return *this;
}

/// Open the index file
void SqlIndexFile::open(bool create, SqlError &error) {
  // flags setup
  const char *flags = create ? "w+b" : "r+b";

  // open file
  this->m_file.open(flags, error);

  if (create) {
    // if creating, setup magic and num_tables

    // write magic
    this->m_file.write((const uint8_t *)SQL_INDEX_FILE_MAGIC,
                       SQL_INDEX_FILE_MAGIC_SIZE, error);
    if (!error.is_ok())
      return;

    // write num_tables
    this->m_file.write((const uint8_t *)&this->num_tables, 1, error);
    if (!error.is_ok())
      return;
  } else {
    // otherwise, validate magic and load tables

    // prep buffer for magic
    char buffer[SQL_INDEX_FILE_MAGIC_SIZE] = {0};

    // read magic
    this->m_file.read((uint8_t *)buffer, SQL_INDEX_FILE_MAGIC_SIZE, error);
    if (!error.is_ok())
      return;

    // validate magic
    for (size_t i = 0; i < SQL_INDEX_FILE_MAGIC_SIZE; i++) {
      if (SQL_INDEX_FILE_MAGIC[i] != buffer[i]) {
        error.set_invalid_file();
        return;
      }
    }

    // read num_tables
    this->m_file.read(&this->num_tables, 1, error);
    if (!error.is_ok())
      return;
  }
}

/// Seek to a table name index
void SqlIndexFile::seek_to_table_name_index(size_t index, SqlError &error) {
  // 1 is the size of the # of tables field
  size_t header_size = SQL_INDEX_FILE_MAGIC_SIZE + 1;
  // 1 is the size of the length field.
  size_t body_offset = (1 + TABLE_NAME_MAX_LENGTH) * index;
  this->m_file.seek(header_size + body_offset, error);
}

/// Get the # of tables in the index.
uint8_t SqlIndexFile::get_num_tables() { return this->num_tables; }

/// Get the index of a table name.
///
/// Returns the index, or -1 if not found.
/// If an error occured, the result is also -1.
int SqlIndexFile::index_of_table_name(
    const SmallString<TABLE_NAME_MAX_LENGTH> &table_name, SqlError &error) {
  SmallString<TABLE_NAME_MAX_LENGTH> temp_name;
  for (size_t i = 0; i < this->num_tables; i++) {
    this->get_table_name(i, temp_name, error);
    if (!error.is_ok())
      return -1;
    if (table_name == temp_name)
      return i;
  }

  return -1;
}

/// Get the table name at the index.
void SqlIndexFile::get_table_name(
    size_t index, SmallString<TABLE_NAME_MAX_LENGTH> &table_name,
    SqlError &error) {
  // seek to position
  this->seek_to_table_name_index(index, error);
  if (!error.is_ok())
    return;

  // read small string
  read_small_string_from_file(table_name, this->m_file, error);
  if (!error.is_ok())
    return;
}

/// Insert the table name at the given index, overwriting it.
void SqlIndexFile::insert_table_name(
    size_t index, const SmallString<TABLE_NAME_MAX_LENGTH> &table_name,
    SqlError &error) {
  // seek to position
  this->seek_to_table_name_index(index, error);
  if (!error.is_ok())
    return;

  // write name
  write_small_string_to_file(table_name, this->m_file, error);
  if (!error.is_ok())
    return;
}

/// Remove the table at the index.
///
/// This function will abort if i >= num_tables.
void SqlIndexFile::remove(size_t i, SqlError &error) {
  assert(i < this->num_tables);

  // Get last name
  SmallString<TABLE_NAME_MAX_LENGTH> last_name;
  this->get_table_name(this->num_tables - 1, last_name, error);
  if (!error.is_ok())
    return;

  // Overwrite
  this->insert_table_name(i, last_name, error);

  // Update num tables
  this->update_num_tables(this->num_tables - 1, error);
}

/// Update num_tables
void SqlIndexFile::update_num_tables(uint8_t new_num_tables, SqlError &error) {
  // seek
  // the num_tables field is right after the magic
  this->m_file.seek(SQL_INDEX_FILE_MAGIC_SIZE, error);
  if (!error.is_ok())
    return;

  // update the field with new data
  this->m_file.write(&new_num_tables, 1, error);
  if (!error.is_ok())
    return;

  // only update memory value if past operations persisted to disk.
  this->num_tables = new_num_tables;
}

/// Returns true if this is closed.
bool SqlIndexFile::is_closed() const { return this->m_file.is_closed(); }

/// Close this file
void SqlIndexFile::close(SqlError &error) { this->m_file.close(error); }

/// Get the file name
const std::string &SqlIndexFile::file_name() const {
  return this->m_file.name();
}

/// remove the table file and close this file.
void SqlIndexFile::remove_file(SqlError &error) {
  this->m_file.remove_file(error);
  if (!error.is_ok())
    return;
}

} // namespace basic_sql