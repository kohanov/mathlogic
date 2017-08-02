#include <iostream>
#include <fstream>

#include "parser.h"
#include "common.h"

int main() {
    std::cout<< R"(Usage: "input_file" "output_file")" <<std::endl;
    std::string input_file, output_file;
    std::cin >> input_file >> output_file;
    std::ifstream in(input_file);
    std::ofstream out(output_file);
    expr_v axioms = parser::load_proof("rules/axioms.txt");
    expr_v assumptions = parse_header(in);
    annotate(in, out, axioms, assumptions);
    in.close();
    out.close();
    return 0;
}