# cmake_minimum_required(VERSION 3.14)
cmake_minimum_required(VERSION 3.10)
project(BasicSql LANGUAGES CXX)

# Convenience vars
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined,signed-integer-overflow -fno-omit-frame-pointer")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

# include(FetchContent)

add_library(
    BasicSql 
    STATIC 
    src/lib.cpp 
    src/ConstStringSlice.cpp 
    src/SqlTokenizer.cpp 
    src/SqlToken.cpp 
    src/SqlParser.cpp
    src/SqlStatement.cpp 
    src/SqlFile.cpp 
    src/SqlError.cpp 
    src/Util.cpp
    src/SqlIndexFile.cpp
    src/SqlTableFile.cpp
    src/tokenizer/SqlType.cpp
    src/SerDe.cpp
    src/SqlValue.cpp
)
target_include_directories(BasicSql PUBLIC include)

add_executable(BasicSqlCli apps/main.cpp)
target_include_directories(BasicSqlCli PUBLIC include)
target_link_libraries(BasicSqlCli PUBLIC BasicSql)

# if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
#    add_subdirectory(tests)
# endif()
