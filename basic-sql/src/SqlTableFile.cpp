/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#include "SqlTableFile.h"

namespace basic_sql {
/// Create a new unopened file
SqlTableFile::SqlTableFile(std::string name)
    : m_file(name), num_columns(0), num_values(0) {}
SqlTableFile::SqlTableFile(SqlTableFile &&other) noexcept
    : m_file(std::move(other.m_file)), num_columns(other.num_columns),
      columns(other.columns), num_values(other.num_values) {}
SqlTableFile &SqlTableFile::operator=(SqlTableFile &&other) {
  SqlError error;
  this->close(error);
  this->m_file = std::move(other.m_file);
  this->num_columns = other.num_columns;
  this->columns = other.columns;
  this->num_values = other.num_values;
  return *this;
}

/// Update the num_values field
void SqlTableFile::update_num_values(uint8_t new_num_values, SqlError &error) {
  // seek to field
  this->m_file.seek(SQL_TABLE_FILE_VALUES_OFFSET, error);
  if (!error.is_ok())
    return;

  // write new data
  this->m_file.write(&new_num_values, 1, error);
  if (!error.is_ok())
    return;

  // update memeory
  this->num_values = new_num_values;
}

/// get the number of values
uint8_t SqlTableFile::get_num_values() { return this->num_values; }

/// Add a column.
void SqlTableFile::add_column(const parser::SqlColumn &column,
                              SqlError &error) {
  // check limit
  if (num_columns > COLUMN_MAX) {
    error.set_limit_reached();
    return;
  }

  // seek to position
  this->seek_to_column_index(this->num_columns, error);
  if (!error.is_ok())
    return;

  // Write name
  write_small_string_to_file(column.name, this->m_file, error);
  if (!error.is_ok())
    return;

  // TODO: return error
  assert(column.type.size < MAX_TYPE_SIZE);

  // Write type
  write_sql_type(column.type, this->m_file, error);

  // update memory
  // TODO: Do i check for size above? can i remove the assert/error?
  assert(this->columns.push(column));

  // update num columns
  this->update_num_columns(this->num_columns + 1, error);
  if (!error.is_ok())
    return;
}

/// seek to a column by index.
///
/// This allows seeking to uninitialized columns so the user can initialize
/// them.
void SqlTableFile::seek_to_column_index(size_t i, SqlError &error) {
  // TODO: Return error?
  assert(i < COLUMN_MAX);

  size_t position = SQL_TABLE_FILE_COLUMN_OFFSET +
                    (SQL_TABLE_FILE_COLUMN_DATA_ELEMENT_SIZE * i);

  // seek to position
  this->m_file.seek(position, error);
  if (!error.is_ok())
    return;
}

/// Update num_columns on file and in mem
void SqlTableFile::update_num_columns(uint8_t new_num_columns,
                                      SqlError &error) {
  // num_columns field is after the magic
  this->m_file.seek(SQL_TABLE_FILE_MAGIC_SIZE, error);
  if (!error.is_ok())
    return;

  // write new data
  this->m_file.write(&new_num_columns, 1, error);
  if (!error.is_ok())
    return;

  // update memeory
  this->num_columns = new_num_columns;
}

/// query rows
void SqlTableFile::query_rows(
    const SmallVec<COLUMN_MAX, SmallString<COLUMN_NAME_MAX_LENGTH>>
        &column_names,
    const parser::SqlWhereClause *where_clause, QueryRowsResult &result,
    SqlError &error) {
  SmallVec<COLUMN_MAX, int> column_name_indicies;

  if (column_names.size() == 0) {
    result.columns = this->columns;
  } else {
    for (size_t i = 0; i < column_names.size(); i++) {
      int index = this->get_index_of_column_name(column_names[i]);
      assert(index != -1);
      column_name_indicies.push(index);
      result.columns.push(this->columns[index]);
    }
  }

  size_t column_index = -1;

  if (where_clause != nullptr) {
    // find where clause column index
    for (size_t j = 0; j < this->columns.size(); j++) {
      if (this->columns[j].name == where_clause->column_name) {
        column_index = j;
      }
    }
  }

  for (size_t i = 0; i < this->num_values; i++) {
    // seek to values
    this->seek_to_value_index(i, error);
    if (!error.is_ok())
      return;

    SmallVec<COLUMN_MAX, SqlValue> row;

    // read row
    this->get_row(i, row, error);
    if (!error.is_ok())
      return;

    if ((where_clause != nullptr &&
         where_clause->value_matches(row[column_index])) ||
        where_clause == nullptr) {

      // append row to result
      if (column_name_indicies.size() == 0) {
        result.rows.push_back(row);
      } else {
        SmallVec<COLUMN_MAX, SqlValue> result_row;
        for (size_t i = 0; i < column_name_indicies.size(); i++) {
          result_row.push(row[column_name_indicies[i]]);
        }
        result.rows.push_back(result_row);
      }
    }
  }
}

/// Get the columns
const SmallVec<COLUMN_MAX, parser::SqlColumn> &
SqlTableFile::get_columns() const {
  return this->columns;
}

/// remove the table file and close this file.
void SqlTableFile::remove_file(SqlError &error) {
  this->m_file.remove_file(error);
  if (!error.is_ok())
    return;
}

/// Returns true if this is closed
bool SqlTableFile::is_closed() { return this->m_file.is_closed(); }

/// Close this file
void SqlTableFile::close(SqlError &error) { this->m_file.close(error); }

/// Get the file name
const std::string &SqlTableFile::file_name() const {
  return this->m_file.name();
}
} // namespace basic_sql