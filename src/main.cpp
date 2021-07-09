#include <iostream>
#include <parser/driver.h>

int main(int argc, char *argv[]) {
    tcc_parser_driver driver;
    if (!driver.parse(argv[1]))
        std::cout << "success" << std::endl;
    else
        std::cout << "fail" << std::endl;
    return 0;
}

