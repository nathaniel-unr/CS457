/// Author: Nathaniel Daniel
/// Date: 10-17-2021

#ifndef _SQL_TABLE_H_
#define _SQL_TABLE_H_

#include "SerDe.h"
#include "SqlFile.h"
#include "SqlStatement.h"
#include "Util.h"
#include <vector>

namespace basic_sql {
/// A buffered row
struct BufferedRow {
  size_t row_index;
  SmallVec<COLUMN_MAX, SqlValue> row;
};

/// The result of querying rows
struct QueryRowsResult {
  SmallVec<COLUMN_MAX, parser::SqlColumn> columns;
  std::vector<SmallVec<COLUMN_MAX, SqlValue>> rows;
};

/// Magic
static const char *SQL_TABLE_FILE_MAGIC = "table";
/// Magic len
static const uint8_t SQL_TABLE_FILE_MAGIC_SIZE = 5;
/// size of column data.
///
/// the size of column data is 1 for the name length, COLUMN_NAME_MAX_LENGTH
/// for the name, 1 for the type size, and 1 for the type.
static const size_t SQL_TABLE_FILE_COLUMN_DATA_ELEMENT_SIZE =
    1 + COLUMN_NAME_MAX_LENGTH + 1 + 1;
/// the offset to the column data
///
/// the column data starts after the num_columns field
static const size_t SQL_TABLE_FILE_COLUMN_OFFSET =
    SQL_TABLE_FILE_MAGIC_SIZE + 1;
/// the offset to the values data
///
/// size of column data field plus size of num_values
static const size_t SQL_TABLE_FILE_VALUES_OFFSET =
    SQL_TABLE_FILE_COLUMN_OFFSET +
    (COLUMN_MAX * SQL_TABLE_FILE_COLUMN_DATA_ELEMENT_SIZE);

/// A SQl table file
class SqlTableFile {
public:
  /// Create a new unopened file
  SqlTableFile(std::string name);
  SqlTableFile(const SqlTableFile &other) = delete;
  SqlTableFile &operator=(SqlTableFile &other) = delete;
  SqlTableFile(SqlTableFile &&other) noexcept;
  SqlTableFile &operator=(SqlTableFile &&other);

  /// Open the table file
  void open(bool create, SqlError &error) {
    // flags setup
    const char *flags = create ? "w+b" : "r+b";

    this->m_file.open(flags, error);

    if (create) {
      // write magic
      this->m_file.write((const uint8_t *)SQL_TABLE_FILE_MAGIC,
                         SQL_TABLE_FILE_MAGIC_SIZE, error);
      if (!error.is_ok())
        return;

      // write # of columns
      this->m_file.write((const uint8_t *)&this->num_columns, 1, error);
      if (!error.is_ok())
        return;

      // zero column data
      this->m_file.write_byte_n(
          0, COLUMN_MAX * SQL_TABLE_FILE_COLUMN_DATA_ELEMENT_SIZE, error);
      if (!error.is_ok())
        return;

      // write # of values
      this->m_file.write((const uint8_t *)&this->num_values, 1, error);
      if (!error.is_ok())
        return;
    } else {
      // read magic
      // sql index magic buffer
      char buffer[SQL_TABLE_FILE_MAGIC_SIZE] = {0};
      this->m_file.read((uint8_t *)buffer, SQL_TABLE_FILE_MAGIC_SIZE, error);
      if (!error.is_ok())
        return;

      // validate magic
      for (size_t i = 0; i < SQL_TABLE_FILE_MAGIC_SIZE; i++) {
        if (buffer[i] != SQL_TABLE_FILE_MAGIC[i]) {
          error.set_invalid_file();
          return;
        }
      }

      // read num_columns
      this->m_file.read(&this->num_columns, 1, error);
      if (!error.is_ok())
        return;

      // load column data
      for (int i = 0; i < this->num_columns; i++) {
        SmallString<COLUMN_NAME_MAX_LENGTH> column_name;
        read_small_string_from_file(column_name, this->m_file, error);
        if (!error.is_ok())
          return;

        parser::SqlType type;
        read_sql_type(type, this->m_file, error);
        if (!error.is_ok())
          return;

        parser::SqlColumn column{
          name : column_name,
          type,
        };
        // TODO: Do i check for size above? can i remove the assert/error?
        assert(this->columns.push(column));
      }

      // read num_values
      // seek to field
      this->m_file.seek(SQL_TABLE_FILE_VALUES_OFFSET, error);
      if (!error.is_ok())
        return;
      // read
      this->m_file.read(&this->num_values, 1, error);
      if (!error.is_ok())
        return;
    }
  }

  /// update rows
  void update_rows(const parser::SqlStatementUpdate &statement,
                   bool in_transaction, size_t &num_modified, SqlError &error) {
    for (size_t row_index = 0; row_index < this->num_values; row_index++) {
      // seek to values
      this->seek_to_value_index(row_index, error);
      if (!error.is_ok())
        return;

      SmallVec<COLUMN_MAX, SqlValue> row;
      size_t column_index = -1;
      size_t update_index = -1;

      // read sql values
      for (size_t j = 0; j < this->columns.size(); j++) {
        // read sql value
        SqlValue value;
        read_sql_value(value, this->columns[j].type.type, this->m_file, error);
        if (!error.is_ok())
          return;

        // append to row
        row.push(value);

        // find where clause column index
        if (this->columns[j].name == statement.where_clause.column_name) {
          column_index = j;
        }

        // find statement column index
        if (this->columns[j].name == statement.column_name) {
          update_index = j;
        }
      }

      // update row
      if (column_index != -1 &&
          statement.where_clause.value_matches(row[column_index])) {
        // TODO: ensure types match
        row[update_index] = statement.value;

        if (in_transaction) {
          this->m_buffered_rows.push_back(BufferedRow{row_index, row});
        } else {
          this->insert(row_index, row, error);
          if (!error.is_ok())
            return;
        }

        num_modified += 1;
      }
    }
  }

  /// delete rows
  void delete_rows(const parser::SqlStatementDelete &statement,
                   size_t &num_deleted, SqlError &error) {
    for (size_t row_index = 0; row_index < this->num_values; row_index++) {
      // seek to values
      this->seek_to_value_index(row_index, error);
      if (!error.is_ok())
        return;

      SmallVec<COLUMN_MAX, SqlValue> row;
      size_t column_index = -1;

      // get row
      this->get_row(row_index, row, error);
      if (!error.is_ok())
        return;

      // find where clause column index
      for (size_t j = 0; j < this->columns.size(); j++) {
        if (this->columns[j].name == statement.where_clause.column_name) {
          column_index = j;
        }
      }

      // delete row
      if (column_index != -1 &&
          statement.where_clause.value_matches(row[column_index])) {
        this->remove_row(row_index, error);
        if (!error.is_ok())
          return;

        num_deleted += 1;
        row_index -= 1;
      }
    }
  }

  /// insert a value at the given index.
  ///
  /// the index cannot exceed num_values, and writing to num_values makes a new
  /// record.
  void insert(size_t index, const SmallVec<COLUMN_MAX, SqlValue> &data,
              SqlError &error) {
    // seek to index
    this->seek_to_value_index(index, error);
    if (!error.is_ok())
      return;

    // write sql values
    for (size_t i = 0; i < data.size(); i++) {
      write_sql_value(data[i], this->m_file, error);
      if (!error.is_ok())
        return;
    }

    // pad remaining empty columns
    for (size_t j = data.size() * MAX_TYPE_SIZE; j < COLUMN_MAX * MAX_TYPE_SIZE;
         j++) {
      uint8_t data = 0;
      this->m_file.write(&data, 1, error);
      if (!error.is_ok())
        return;
    }
  }

  /// Remove a row at a given index
  ///
  /// the index cannot exceed num_values
  void remove_row(size_t index, SqlError &error) {
    if (this->num_values != 0) {
      SmallVec<COLUMN_MAX, SqlValue> row;

      // get last entry
      this->get_row(this->num_values - 1, row, error);
      if (!error.is_ok())
        return;

      // overwrite entry
      this->insert(index, row, error);
      if (!error.is_ok())
        return;
    }

    // shrink num_values
    this->update_num_values(this->num_values - 1, error);
    if (!error.is_ok())
      return;
  }

  /// Get a row at a given index
  ///
  /// the index cannot exceed num_values
  void get_row(size_t index, SmallVec<COLUMN_MAX, SqlValue> &row,
               SqlError &error) {
    // seek to index
    this->seek_to_value_index(index, error);
    if (!error.is_ok())
      return;

    // read sql values
    for (size_t j = 0; j < this->columns.size(); j++) {
      // read sql value
      SqlValue value;
      read_sql_value(value, this->columns[j].type.type, this->m_file, error);
      if (!error.is_ok())
        return;

      // append to row
      row.push(value);
    }
  }

  /// Seek to a value by index
  ///
  /// the index cannot exceed num_values, and writing to num_values makes a new
  /// record.
  void seek_to_value_index(size_t index, SqlError &error) {
    assert(index <= this->num_values);

    // size of values offset plus size of num_values
    size_t position =
        SQL_TABLE_FILE_VALUES_OFFSET + 1 + (index * MAX_TYPE_SIZE * COLUMN_MAX);
    this->m_file.seek(position, error);
    if (!error.is_ok())
      return;
  }

  /// Get the index of a column name.
  ///
  /// Returns -1 if it could not be found.
  int get_index_of_column_name(
      const SmallString<COLUMN_NAME_MAX_LENGTH> &name) {
    for (size_t i = 0; i < this->columns.size(); i++) {
      if (this->columns[i].name == name)
        return i;
    }

    return -1;
  }

  /// Commit buffered values
  void commit(SqlError &error) {
    for (size_t i = 0; i < this->m_buffered_rows.size(); i++) {
      // seek to values
      this->seek_to_value_index(this->m_buffered_rows[i].row_index, error);
      this->insert(this->m_buffered_rows[i].row_index,
                   this->m_buffered_rows[i].row, error);
      if (!error.is_ok())
        return;
    }

    this->clear_buffered_rows();
    this->m_file.flush(error);
  }

  /// Clear buffered rows
  void clear_buffered_rows() { this->m_buffered_rows.clear(); }

  /// Update the num_values field
  void update_num_values(uint8_t new_num_values, SqlError &error);

  /// get the number of values
  uint8_t get_num_values();

  /// Add a column.
  void add_column(const parser::SqlColumn &column, SqlError &error);

  /// seek to a column by index.
  ///
  /// This allows seeking to uninitialized columns so the user can initialize
  /// them.
  void seek_to_column_index(size_t i, SqlError &error);

  /// Update num_columns on file and in mem
  void update_num_columns(uint8_t new_num_columns, SqlError &error);

  /// query rows
  void
  query_rows(const SmallVec<COLUMN_MAX, SmallString<COLUMN_NAME_MAX_LENGTH>>
                 &column_names,
             const parser::SqlWhereClause *where_clause,
             QueryRowsResult &result, SqlError &error);

  /// Get the columns
  const SmallVec<COLUMN_MAX, parser::SqlColumn> &get_columns() const;

  /// remove the table file and close this file.
  void remove_file(SqlError &error);

  /// Returns true if this is closed
  bool is_closed();

  /// Close this file
  void close(SqlError &error);

  /// Get the file name
  const std::string &file_name() const;

private:
  SqlFile m_file;
  uint8_t num_columns;
  uint8_t num_values;
  SmallVec<COLUMN_MAX, parser::SqlColumn> columns;

  std::vector<BufferedRow> m_buffered_rows;
};
} // namespace basic_sql
#endif
