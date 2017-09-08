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

    std::vector<p_expr> expressions;
    std::string s;
    while (std::getline(in, s)) {
        expressions.emplace_back(p_expr(parser::parse_expr(s)));
    }

    expr_v result = deduction(expressions, axioms, assumptions);
    for (auto &res : result) {
        out << res->str << std::endl;
    }
    in.close();
    out.close();
    return 0;
}