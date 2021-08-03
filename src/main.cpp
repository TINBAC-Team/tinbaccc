#include <iostream>
#include <getopt.h>

#include <parser/tcc-sy-driver.hh>

static const struct option long_options[] = {
        {"generate-assembly", no_argument,       NULL, 'S'},
        {"output",            required_argument, NULL, 'o'},
        {"output-asm-tmp",    required_argument, NULL, 't'},
        {"optimization",      required_argument, NULL, 'O'},
        {"output-ast",        required_argument, NULL, 'a'},
        {"output-ir",         required_argument, NULL, 'i'},
        {NULL,                no_argument,       NULL, 0}
};

int main(int argc, char *argv[]) {
    int ch;
    const char *ast_path = nullptr;
    const char *ir_path = nullptr;
    const char *asm_path = nullptr;
    const char *asm_tmp_path = nullptr;
    while ((ch = getopt_long(argc, argv, "So:O:a:i:s:t:", long_options, NULL)) != -1) {
        switch (ch) {
            case 'S':
            case 'O':
                break;
            case 'o':
                asm_path = optarg;
                break;
            case 'a':
                ast_path = optarg;
                break;
            case 'i':
                ir_path = optarg;
                break;
            case 't':
                asm_tmp_path = optarg;
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

    driver.generate_ir();
    driver.process_ir();
    if(ir_path)
        driver.print_ir(ir_path);
    driver.generate_asm();
    if (asm_tmp_path)
        driver.print_asm(asm_tmp_path);
    driver.process_asm();
    driver.print_asm(asm_path);
    return 0;
}

