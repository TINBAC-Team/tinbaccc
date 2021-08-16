cmake_minimum_required(VERSION 3.10)
project(tinbaccc VERSION 0.1)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(BISON)
find_package(FLEX)

include_directories(include)
bison_target(tcc_sy_parser src/parser/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/tcc-sy-parser.cpp
#        COMPILE_FLAGS -Wcounterexamples
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
        src/ast/validation.cpp
        src/ast/codegen.cpp)

set(SYMTAB_SOURCES
        src/ast/symtab.cpp)

set(IR_SOURCES
        src/ir/ir.cpp
        src/ir/print.cpp
        src/ir/codegen.cpp
        src/ir/codegen_simd.cpp
        src/ir/ir_simd.cpp)

set(IR_PASSES_SOURCES
        src/ir/passes/dominator.cpp
        src/ir/passes/function_ops.cpp
        src/ir/passes/fixups.cpp
        src/ir/passes/rpo.cpp
        src/ir/passes/gvn_gcm.cpp
        src/ir/passes/basicblock_ops.cpp
        src/ir/passes/tail_call_elimination.cpp
        src/ir/passes/loop_detect.cpp
        src/ir/passes/adce.cpp
        src/ir/passes/loop_unrolling.cpp
        src/ir/passes/eliminate_load.cpp
        src/ir/passes/strength_reduction.cpp
        src/ir/passes/vectorization.cpp
        src/ir/passes/loop.cpp
        src/ir/passes/side_effect.cpp)

set(ASM_SOURCES
        src/asm_arm/instructions.cpp
        src/asm_arm/builder.cpp
        src/asm_arm/print.cpp
        src/asm_arm/registers.cpp
        src/asm_arm/function_fixup.cpp
        src/asm_arm/optimization.cpp)

add_executable(tinbaccc src/main.cpp
        ${PARSER_SOURCES}
        ${AST_SOURCES}
        ${SYMTAB_SOURCES}
        ${IR_SOURCES}
        ${IR_PASSES_SOURCES}
        ${ASM_SOURCES})


target_include_directories(tinbaccc
        PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
