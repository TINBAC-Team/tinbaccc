#include <parser/tcc-sy-driver.hh>

tcc_sy_driver::tcc_sy_driver()
        : trace_scanning(false), trace_parsing(false) {
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
