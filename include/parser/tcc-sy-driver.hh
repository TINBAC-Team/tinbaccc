#ifndef TINBACCC_TCC_SY_DRIVER_HH
#define TINBACCC_TCC_SY_DRIVER_HH
# include <string>
# include <map>
#include <memory>
# include "tcc-sy-parser.hh"

namespace ast {
    class CompUnit;
}

namespace ir {
    class Module;
}

namespace asm_arm {
    class Module;
}

// Tell Flex the lexer's prototype ...
# define YY_DECL \
  yy::tcc_sy_parser::symbol_type yylex (tcc_sy_driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;
// Conducting the whole scanning and parsing.
class tcc_sy_driver
{
public:
    tcc_sy_driver ();
    virtual ~tcc_sy_driver ();

    // Final AST
    std::unique_ptr<ast::CompUnit> comp_unit;
    // IR
    ir::Module *module;
    asm_arm::Module *asm_module;
    // Handling the scanner.
    void scan_begin ();
    void scan_end ();
    bool trace_scanning;
    // Run the parser on file F.
    // Return 0 on success.
    int parse (const std::string& f);
    // The name of the file being parsed.
    // Used later to pass the file name to the location tracker.
    std::string file;
    // Whether parser traces should be generated.
    bool trace_parsing;
    // Error handling.
    void error (const yy::location& l, const std::string& m);
    void error (const std::string& m);
    void print_ast(const char *path);
    void validate_ast();
    void generate_ir();
    void print_ir(const char *path);
    void process_ir();
    void generate_asm();
    void print_asm(const char *path);
    void process_asm();
};
#endif //TINBACCC_TCC_SY_DRIVER_HH
