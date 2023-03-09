#include "BasicSql.h"
#include <catch2/catch.hpp>
#include <iostream>

using basic_sql::ConstStringSlice;
using basic_sql::tokenizer::SqlIdentifier;
using basic_sql::tokenizer::SqlIntegerLiteral;
using basic_sql::tokenizer::SqlKeyword;
using basic_sql::tokenizer::SqlToken;
using basic_sql::tokenizer::SqlTokenizer;
using basic_sql::tokenizer::SqlTokenizerError;
using basic_sql::tokenizer::SqlType;

TEST_CASE("NullConstStringSlice", "[main]") {
  ConstStringSlice null_slice(nullptr, 0);
  ConstStringSlice null_slice1("");

  SECTION("a nullptr is a valid ConstStringSlice") {
    REQUIRE(null_slice.is_empty());
    REQUIRE(null_slice.size() == 0);
  }

  SECTION("an empty c string is a valid ConstStringSlice") {
    REQUIRE(null_slice1.is_empty());
    REQUIRE(null_slice1.size() == 0);
  }

  REQUIRE(null_slice == null_slice1);
}

TEST_CASE("Project1Tokenizer", "[main]") {
  SECTION("tokenize 'CREATE DATABASE db_1;'") {
    std::string sql("CREATE DATABASE db_1;");
    std::vector<SqlToken> expected_tokens{
        SqlToken(SqlKeyword::CREATE),
        SqlToken(SqlKeyword::DATABASE),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("db_1"),
        }),
        SqlToken::semicolon(),
    };

    SqlTokenizer tokenizer(sql);
    std::vector<SqlToken> tokens;
    SqlTokenizerError e;
    tokenizer.tokenize(tokens, e);

    INFO(e.message);
    REQUIRE(e.is_ok());
    REQUIRE(expected_tokens == tokens);
  }

  SECTION("tokenize 'DROP DATABASE db_2;'") {
    std::string sql("DROP DATABASE db_2;");
    std::vector<SqlToken> expected_tokens{
        SqlToken(SqlKeyword::DROP),
        SqlToken(SqlKeyword::DATABASE),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("db_2"),
        }),
        SqlToken::semicolon(),
    };

    SqlTokenizer tokenizer(sql);
    std::vector<SqlToken> tokens;
    SqlTokenizerError e;
    tokenizer.tokenize(tokens, e);

    INFO(e.message);
    REQUIRE(e.is_ok());
    REQUIRE(expected_tokens == tokens);
  }

  SECTION("tokenize 'USE db_1;'") {
    std::string sql("USE db_1;");
    std::vector<SqlToken> expected_tokens{
        SqlToken(SqlKeyword::USE),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("db_1"),
        }),
        SqlToken::semicolon(),
    };

    SqlTokenizer tokenizer(sql);
    std::vector<SqlToken> tokens;
    SqlTokenizerError e;
    tokenizer.tokenize(tokens, e);

    INFO(e.message);
    REQUIRE(e.is_ok());
    REQUIRE(expected_tokens == tokens);
  }

  SECTION("tokenize 'CREATE TABLE tbl_1 (a1 int, a2 varchar(20));'") {
    std::string sql("CREATE TABLE tbl_1 (a1 int, a2 varchar(20));");
    std::vector<SqlToken> expected_tokens{
        SqlToken(SqlKeyword::CREATE),
        SqlToken(SqlKeyword::TABLE),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("tbl_1"),
        }),
        SqlToken::left_parenthesis(),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("a1"),
        }),
        SqlToken(SqlType::INT),
        SqlToken::comma(),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("a2"),
        }),
        SqlToken(SqlType::VARCHAR),
        SqlToken::left_parenthesis(),
        SqlToken(SqlIntegerLiteral{value : 20}),
        SqlToken::right_parenthesis(),
        SqlToken::right_parenthesis(),
        SqlToken::semicolon(),
    };

    SqlTokenizer tokenizer(sql);
    std::vector<SqlToken> tokens;
    SqlTokenizerError e;
    tokenizer.tokenize(tokens, e);

    INFO(e.message);
    REQUIRE(e.is_ok());
    REQUIRE(expected_tokens == tokens);
  }

  SECTION("tokenize 'CREATE TABLE tbl_1 (a3 float, a4 char(20));'") {
    std::string sql("CREATE TABLE tbl_1 (a3 float, a4 char(20));");
    std::vector<SqlToken> expected_tokens{
        SqlToken(SqlKeyword::CREATE),
        SqlToken(SqlKeyword::TABLE),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("tbl_1"),
        }),
        SqlToken::left_parenthesis(),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("a3"),
        }),
        SqlToken(SqlType::FLOAT),
        SqlToken::comma(),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("a4"),
        }),
        SqlToken(SqlType::CHAR),
        SqlToken::left_parenthesis(),
        SqlToken(SqlIntegerLiteral{value : 20}),
        SqlToken::right_parenthesis(),
        SqlToken::right_parenthesis(),
        SqlToken::semicolon(),
    };

    SqlTokenizer tokenizer(sql);
    std::vector<SqlToken> tokens;
    SqlTokenizerError e;
    tokenizer.tokenize(tokens, e);

    INFO(e.message);
    REQUIRE(e.is_ok());
    REQUIRE(expected_tokens == tokens);
  }

  SECTION("tokenize 'DROP TABLE tbl_1;'") {
    std::string sql("DROP TABLE tbl_1;");
    std::vector<SqlToken> expected_tokens{
        SqlToken(SqlKeyword::DROP),
        SqlToken(SqlKeyword::TABLE),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("tbl_1"),
        }),
        SqlToken::semicolon(),
    };

    SqlTokenizer tokenizer(sql);
    std::vector<SqlToken> tokens;
    SqlTokenizerError e;
    tokenizer.tokenize(tokens, e);

    INFO(e.message);
    REQUIRE(e.is_ok());
    REQUIRE(expected_tokens == tokens);
  }

  SECTION("tokenize 'SELECT * FROM tbl_1;'") {
    std::string sql("SELECT * FROM tbl_1;");
    std::vector<SqlToken> expected_tokens{
        SqlToken(SqlKeyword::SELECT),
        SqlToken::asterisk(),
        SqlToken(SqlKeyword::FROM),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("tbl_1"),
        }),
        SqlToken::semicolon(),
    };

    SqlTokenizer tokenizer(sql);
    std::vector<SqlToken> tokens;
    SqlTokenizerError e;
    tokenizer.tokenize(tokens, e);

    INFO(e.message);
    REQUIRE(e.is_ok());
    REQUIRE(expected_tokens == tokens);
  }

  SECTION("tokenize 'ALTER TABLE tbl_1 ADD a3 float;'") {
    std::string sql("ALTER TABLE tbl_1 ADD a3 float;");
    std::vector<SqlToken> expected_tokens{
        SqlToken(SqlKeyword::ALTER),
        SqlToken(SqlKeyword::TABLE),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("tbl_1"),
        }),
        SqlToken(SqlKeyword::ADD),
        SqlToken(SqlIdentifier{
          value : ConstStringSlice("a3"),
        }),
        SqlToken(SqlType::FLOAT),
        SqlToken::semicolon(),
    };

    SqlTokenizer tokenizer(sql);
    std::vector<SqlToken> tokens;
    SqlTokenizerError e;
    tokenizer.tokenize(tokens, e);

    INFO(e.message);
    REQUIRE(e.is_ok());
    REQUIRE(expected_tokens == tokens);
  }
}
