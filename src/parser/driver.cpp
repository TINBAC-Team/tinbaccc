#include <parser/driver.h>
#include <tcc-sy-parser.h>

tcc_parser_driver::tcc_parser_driver()
        : trace_scanning(false), trace_parsing(false) {
}

tcc_parser_driver::~tcc_parser_driver() {
}

int tcc_parser_driver::parse(const std::string &f) {
    file = f;
    scan_begin();
    yy::tcc_sy_parser parser(*this);
    parser.set_debug_level(trace_parsing);
    int res = parser.parse();
    scan_end();
    return res;
}

void tcc_parser_driver::error(const yy::location &l, const std::string &m) {
    std::cerr << l << ": " << m << std::endl;
}

void tcc_parser_driver::error(const std::string &m) {
    std::cerr << m << std::endl;
}
