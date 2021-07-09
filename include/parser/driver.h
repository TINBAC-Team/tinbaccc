#ifndef TINBACCC_DRIVER_H
#define TINBACCC_DRIVER_H
# include <string>
# include <map>
# include "tcc-sy-parser.h"
// Tell Flex the lexer's prototype ...
# define YY_DECL \
  yy::tcc_sy_parser::symbol_type yylex (tcc_parser_driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;
// Conducting the whole scanning and parsing of Calc++.
class tcc_parser_driver
{
public:
    tcc_parser_driver ();
    virtual ~tcc_parser_driver ();

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
};
#endif //TINBACCC_DRIVER_H
