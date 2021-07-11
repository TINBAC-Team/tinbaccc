cmake_minimum_required(VERSION 3.10)
project(tinbaccc VERSION 0.1)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(BISON)
find_package(FLEX)

include_directories(include)
bison_target(tcc_sy_parser src/parser/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/tcc-sy-parser.cpp
        COMPILE_FLAGS -Wcounterexamples
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/tcc-sy-parser.hh)
flex_target(tcc_sy_scanner src/parser/scanner.ll  ${CMAKE_CURRENT_BINARY_DIR}/tcc-sy-scanner.cpp
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/tcc-sy-scanner.hh)

add_flex_bison_dependency(tcc_sy_scanner tcc_sy_parser)

set(PARSER_SOURCES
        ${BISON_tcc_sy_parser_OUTPUTS}
        ${FLEX_tcc_sy_scanner_OUTPUTS}
        src/parser/tcc-sy-driver.cpp)

set(AST_SOURCES
        src/ast/ast.cpp
        src/ast/print.cpp
        src/ast/validation.cpp)

set(SYMTAB_SOURCES
        src/ast/symtab.cpp)

add_executable(tinbaccc src/main.cpp
        ${PARSER_SOURCES}
        ${AST_SOURCES}
        ${SYMTAB_SOURCES})


target_include_directories(tinbaccc
        PRIVATE ${CMAKE_CURRENT_BINARY_DIR})