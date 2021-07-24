#include <parser/tcc-sy-driver.hh>
#include <ast/ast.h>
#include <ast/validation.h>
#include <ir/ir.h>
#include <asm_arm/builder.h>
#include <fstream>

tcc_sy_driver::tcc_sy_driver()
        : trace_scanning(false), trace_parsing(false) {
    comp_unit = std::make_unique<ast::CompUnit>();
}

tcc_sy_driver::~tcc_sy_driver() {
}

int tcc_sy_driver::parse(const std::string &f) {
    file = f;
    scan_begin();
    yy::tcc_sy_parser parser(*this);
    parser.set_debug_level(trace_parsing);
    int res = parser.parse();
    scan_end();
    return res;
}

void tcc_sy_driver::error(const yy::location &l, const std::string &m) {
    std::cerr << l << ": " << m << std::endl;
}

void tcc_sy_driver::error(const std::string &m) {
    std::cerr << m << std::endl;
}

void tcc_sy_driver::print_ast(const char *path) {
    std::ofstream ofs(path, std::ofstream::out);
    if (!ofs.is_open()) {
        std::cerr << "Failed to open dot\n";
    }
    comp_unit->print(ofs);
    ofs.close();
}

void tcc_sy_driver::validate_ast() {
    ast::ValidationContext ctx;
    comp_unit->validate(ctx);
}

void tcc_sy_driver::generate_ir() {
    module = new ir::Module;
    ir::IRBuilder builder(module);
    comp_unit->codegen(builder);
}

void tcc_sy_driver::print_ir(const char *path) {
    if(path) {
        std::ofstream ofs(path, std::ofstream::out);
        if (!ofs.is_open()) {
            std::cerr << "Failed to open ir\n";
        }
        ofs << *module;
        ofs.close();
    }
}

void tcc_sy_driver::generate_asm() {
    asm_module = new asm_arm::Module();
    asm_arm::Builder builder(asm_module);
    module->codegen(builder);
}

void tcc_sy_driver::print_asm(const char *path) {
    if (path) {
        std::ofstream ofs(path, std::ofstream::out);
        if (!ofs.is_open())
            std::cerr << "Failed to open asm path!\n";
        asm_module->print(ofs);
        ofs.close();
    } else {
        asm_module->print(std::cout);
    }
}
