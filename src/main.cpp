#include <iostream>
#include <getopt.h>
#include <parser/tcc-sy-driver.hh>

static const struct option long_options[] = {
        {"generate-assembly", no_argument,       NULL, 'S'},
        {"output",            required_argument, NULL, 'o'},
        {"optimization",      required_argument, NULL, 'O'},
        {"output-ast",        required_argument, NULL, 'a'},
        {NULL,                no_argument,       NULL, 0}
};

int main(int argc, char *argv[]) {
    int ch;
    const char *ast_path = nullptr;
    while ((ch = getopt_long(argc, argv, "So:O:a:", long_options, NULL)) != -1) {
        switch (ch) {
            case 'S':
            case 'O':
                break;
            case 'o':
                break;
            case 'a':
                ast_path = optarg;
                break;
            default:
                return -1;
        }
    }

    tcc_sy_driver driver;

    if (optind == argc) {
        std::cerr << "no input file specified." << std::endl;
        return 0;
    }

    std::cout << "parsing " << argv[optind] << "..." << std::endl;
    if (!driver.parse(argv[optind])) {
        std::cout << "success" << std::endl;
    } else {
        std::cout << "fail" << std::endl;
        return -1;
    }

    driver.validate_ast();

    if (ast_path)
        driver.print_ast(ast_path);

    driver.generate_code();
    return 0;
}

