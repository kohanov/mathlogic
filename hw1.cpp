#include <iostream>
#include <fstream>

#include "parser.h"

void annotate(std::ifstream &in, std::ofstream &out, const expr_container &axioms, const expr_container &assumptions) {
    std::vector<std::pair<std::unique_ptr<expr>, int>> good;
    int line = 1;
    std::string s;
    parser p;
    while (std::getline(in, s)) {
        std::unique_ptr<expr> expression(p.parse(s));
        out << "(" << line << ") " << s << " (";
        bool ok = true;
        int matching = parser::is_axiom(expression.get(), axioms);
        if (matching)
            out << "Сх. акс. " << matching;
        else {
            int assumption_number = 0;
            for (int i = 0; i < assumptions.size(); i++) {
                name_map empty;
                if (parser::match(expression.get(), assumptions[i].get(), empty)) {
                    assumption_number = i + 1;
                    break;
                }
            }
            if (assumption_number) {
                out << "Предп. " << std::to_string(assumption_number);
            } else {
                bool MP = false;
                int iMP, jMP;
                for (int j = good.size() - 1; j > -1; j--) {
                    if (good[j].first->type == EXPR_TYPE::IMPL) {
                        for (int i = good.size() - 1; i > -1; i--) {
                            if (i == j)
                                continue;
                            name_map empty;
                            impl *temp = dynamic_cast<impl *>(good[j].first.get());
                            if (parser::match(good[i].first.get(), temp->left.get(), empty, false)) {
                                empty.clear();
                                if (parser::match(expression.get(), temp->right.get(), empty, false)) {
                                    MP = true;
                                    iMP = good[i].second;
                                    jMP = good[j].second;
                                    break;
                                }
                            }
                        }
                        if (MP)
                            break;
                    }
                }
                if (MP)
                    out << "M.P. " << std::to_string(iMP) << ", " << std::to_string(jMP);
                else
                    out << "Не доказано";
                ok = MP;
            }
        }
        out << ")" << std::endl;
        if (ok)
            good.push_back(std::make_pair(std::unique_ptr<expr>(expression.release()), line));
        line++;
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
    if (pos > 0) {
        while (true) {
            const unsigned long long cur = pos2;
            pos2 = s.find(',', cur);
            assumptions.push_back(
                    std::unique_ptr<expr>(p.parse(s.substr(cur, (pos2 == std::string::npos ? pos : pos2) - cur))));
            if (pos2 == std::string::npos)
                break;
            pos2++;
        }
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
    annotate(file, out, axioms, assumptions);
    file.close();
    out.close();
    return 0;
}