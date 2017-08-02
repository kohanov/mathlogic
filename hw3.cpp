#include <iostream>
#include <fstream>

#include "parser.h"
#include "proof.h"
#include "common.h"

int main() {
    std::cout<< R"(Usage: "input_file" "output_file")" <<std::endl;
    std::string input_file, output_file;
    std::cin >> input_file >> output_file;
    std::ifstream in(input_file);
    std::string s;
    std::getline(in, s);
    in.close();

    p_expr rule(parser::parse_expr(s));
    expr_v axioms = parser::load_proof("rules/axioms.txt");
    var_replace substitution;
    expr::match(rule.get(), rule.get(), false);
    uint64_t count = 1ULL << (expr::data.size());
    for (auto &var : expr::data) {
        substitution.insert(std::make_pair(var.first, false));
    }
    for (long long i = 0; i < count; i++) {
        uint64_t pos = 0ULL;
        for (auto &var : substitution) {
            var.second = static_cast<bool>((1L << pos) & i);
            pos++;
        }
        if (!rule->exec(substitution)) {
            std::ofstream out(output_file);
            out << "Высказывание ложно при ";
            for (auto it = substitution.begin(); it != substitution.end(); it++) {
                if (it != substitution.begin())
                    out << ",";
                out << it->first << '=' << (it->second ? "И" : "Л");
            }
            out << std::endl;
            in.close();
            out.close();
            return 0;
        }
    }
    std::vector<proof> proofs;
    for (long long i = 0; i < count; i++) {
        unsigned long long pos = substitution.size() - 1;
        proof new_proof;
        for (auto &it : substitution) {
            auto is_true = static_cast<bool>((1L << pos) & i);
            if (is_true) {
                new_proof.assumptions.emplace_back(new var(it.first));
            } else {
                new_proof.assumptions.emplace_back(new neg(p_expr(new var(it.first))));
            }
            pos--;
        }

        if (!proof::generate_proof(new_proof, new_proof.assumptions, rule)) {
            std::cout << "Something went wrong" << std::endl;
        }
        proofs.push_back(std::move(new_proof));
    }
    for (int i = 0; i < substitution.size(); i++) {
        std::cout<<"Step " << (i + 1) << " of " << (substitution.size())<<std::endl;
        std::vector<proof> new_proofs;
        for (int j = 0; j < proofs.size(); j += 2) {
            p_expr var = proofs[j + 1].assumptions[proofs[j + 1].assumptions.size() - 1];
            proofs[j].expressions = std::move(deduction(proofs[j].expressions, axioms, proofs[j].assumptions));
            proofs[j + 1].expressions = std::move(deduction(proofs[j + 1].expressions, axioms, proofs[j + 1].assumptions));
            proof::merge_proofs(proofs[j], proofs[j + 1], axioms, var, rule);
            proofs[j].assumptions = std::move(proofs[j + 1].assumptions);
            new_proofs.push_back(std::move(proofs[j]));
        }
        proofs = std::move(new_proofs);
    }
    std::cout << "Printing result...\n";
    std::ofstream out(output_file);
    annotate(out, axioms, proofs[0].expressions, {});
    out.close();
    return 0;
}