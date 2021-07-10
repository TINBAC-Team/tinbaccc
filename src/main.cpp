#include <iostream>
#include <parser/tcc-sy-driver.hh>

int main(int argc, char *argv[]) {
    tcc_sy_driver driver;
    if(argc <= 1)
    {
        std::cerr << "Please specify an input!"<< std::endl;
        return 0;
    }
    if (!driver.parse(argv[1]))
        std::cout << "success" << std::endl;
    else
        std::cout << "fail" << std::endl;
    return 0;
}

