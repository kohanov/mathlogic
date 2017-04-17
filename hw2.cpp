#include <iostream>
#include <fstream>

#include "parser.h"

void deduction(std::ifstream &in, std::ofstream &out, const expr_container &axioms, const expr_container &assumptions) {
    std::vector<std::unique_ptr<expr>> expressions;
    const std::string rule = assumptions.back().get()->str();
    std::vector<std::string> lemma = {
            "A->(A->A)",
            "(A->(A->A))->(A->((A->A)->A))->(A->A)",
            "(A->((A->A)->A))->(A->A)",
            "(A->((A->A)->A))",
            "A->A"};
    std::string s;
    parser p;
    while (std::getline(in, s)) {
        expressions.push_back(std::unique_ptr<expr>(p.parse(s)));
        expr *expression = expressions.back().get();
        s = expression->str();
        int matching = parser::is_axiom(expression, axioms);
        if (!matching) {
            for (int i = 0; i < assumptions.size() - 1; i++) {
                name_map empty;
                if (parser::match(expression, assumptions[i].get(), empty, false)) {
                    matching = true;
                    break;
                }
            }
        }
        if (matching) {
            out << s << std::endl;
            out << s << "->(" << rule << "->" << s << ")" << std::endl;
            out << rule << "->" << s << std::endl;
            continue;
        }
        name_map empty;
        if (parser::match(expression, assumptions.back().get(), empty, false)) {
            for (auto &line: lemma) {
                for (char c : line) {
                    if (c == 'A')
                        out << s;
                    else
                        out << c;
                }
                out << std::endl;
            }
            continue;
        }
        bool MP = false;
        std::string dj;
        for (int j = expressions.size() - 2; j > -1; j--) {
            if (expressions[j]->type == EXPR_TYPE::IMPL) {
                for (int i = expressions.size() - 2; i > -1; i--) {
                    if (i == j)
                        continue;
                    empty.clear();
                    impl *temp = dynamic_cast<impl *>(expressions[j].get());
                    if (parser::match(expressions[i].get(), temp->left.get(), empty, false)) {
                        empty.clear();
                        if (parser::match(expression, temp->right.get(), empty, false)) {
                            MP = true;
                            dj = expressions[i].get()->str();
                            out << "(" << rule << "->" << dj << ")->((" << rule << "->(" << dj << "->" << s << "))->("
                                << rule << "->" << s << "))" << std::endl;
                            out << "((" << rule << "->(" << dj << "->" << s << "))->(" << rule << "->" << s << "))"
                                << std::endl;
                            out << rule << "->" << s << std::endl;
                            break;
                        }
                    }
                }
                if (MP)
                    break;
            }
        }
        if (!MP)
            std::cout << "Error" << std::endl;
    }
}

expr_container read_axioms() {
    expr_container axioms;
    std::ifstream in("axioms.txt");
    std::string s = "";
    parser p;
    while (std::getline(in, s)) {
        axioms.push_back(std::unique_ptr<expr>(p.parse(s)));
    }
    in.close();
    return axioms;
}

expr_container parse_header(std::ifstream &in) {
    expr_container assumptions;
    std::string s;
    std::getline(in, s);
    const unsigned long long pos = s.find("|-");
    unsigned long long pos2 = 0;
    parser p;
    while (true) {
        const unsigned long long cur = pos2;
        pos2 = s.find(',', cur);
        assumptions.push_back(
                std::unique_ptr<expr>(p.parse(s.substr(cur, (pos2 == std::string::npos ? pos : pos2) - cur))));
        if (pos2 == std::string::npos)
            break;
        pos2++;
    }
    std::unique_ptr<expr> result(p.parse(s.substr(pos + 2)));
    return assumptions;
}

int main() {
    std::string input_file, output_file;
    std::cin >> input_file >> output_file;
    std::ifstream file(input_file);
    std::ofstream out(output_file);
    expr_container axioms = read_axioms();
    expr_container assumptions = parse_header(file);
    deduction(file, out, axioms, assumptions);
    file.close();
    out.close();
    return 0;
}