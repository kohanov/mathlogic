#include "parser.h"
#include <iostream>

#ifndef MATHLOGIC_COMMON_H
#define MATHLOGIC_COMMON_H

expr_v deduction(const expr_v &expressions, const expr_v &axioms, expr_v &assumptions) {
    assert(assumptions.size() > 0);
    p_expr assumption(assumptions[assumptions.size() - 1]);
    assumptions.pop_back();
    expr_v result;
    std::vector<std::pair<p_expr, uint32_t >> good;
    uint32_t line = 0;
    for (auto &expression : expressions) {
        line++;
        auto check_pos = expr::check_expr(axioms, assumptions, good, expression);
        bool match_a = expr::match(expression.get(), assumption.get(), false);
        if (check_pos.first == 0 && check_pos.second == 0 && !match_a) {
            std::cout << "Do not proved: " << expression->term << std::endl;
            for (auto &expression2 : expressions) {
                std::cout << expression2->term << std::endl;
            }
            continue;
        }
        good.emplace_back(std::make_pair(expression, line));
        if (match_a) {
            for (auto &rule : parser::load_proof("rules/AimplA.txt")) {
                result.push_back(parser::change(rule, {expression}));
            }
            continue;
        }
        if (check_pos.first == 0 || check_pos.second == 0) {
            // axiom or assumption
            result.emplace_back(expression);
            result.emplace_back(p_expr(new impl(expression, p_expr(
                    new impl(assumption, expression)))));
            result.emplace_back(p_expr(new impl(assumption, expression)));
            continue;
        }
        // Modus ponens
        expr_v double_mp;
        double_mp.push_back(parser::parse_expr("(A->B)->((A->(B->C))->(A->C))"));
        double_mp.push_back(parser::parse_expr("((A->(B->C))->(A->C))"));
        double_mp.push_back(parser::parse_expr("A->C"));
        for (auto &exp : double_mp) {
            result.push_back(parser::change(exp, {assumption, good[check_pos.first - 1].first, expression}));
        }
    }
    return result;
}

void annotate(std::ofstream &out, const expr_v &axioms, const expr_v &expressions, const expr_v &assumptions) {
    std::vector<std::pair<p_expr, uint32_t >> good;
    uint32_t line = 1;
    for (auto &expression : expressions) {
        out << "(" << line << ") " << expression->term << " (";
        auto check_pos = expr::check_expr(axioms, assumptions, good, expression);
        if (check_pos.second == 0) {
            if (check_pos.first > 0) {
                out << "Сх. акс. " << check_pos.first;
                good.emplace_back(std::make_pair(expression, line));
            } else {
                out << "Не доказано";
            }
        } else {
            if (check_pos.first > 0)
                out << "M.P. " << check_pos.first << ", " << check_pos.second;
            else
                out << "Предп. " << check_pos.second;
            good.emplace_back(std::make_pair(expression, line));
        }
        out << (line == expressions.size() ? ")" : ")\n");
        line++;
    }
}

void annotate(std::ifstream &in, std::ofstream &out, const expr_v &axioms, const expr_v &assumptions) {
    std::string s;
    expr_v expressions;
    while (std::getline(in, s)) {
        expressions.emplace_back(parser::parse_expr(s));
    }
    return annotate(out, axioms, expressions, assumptions);
}

expr_v parse_header(std::ifstream &in) {
    expr_v assumptions;
    std::string s;
    std::getline(in, s);
    const uint64_t pos = s.find("|-");
    uint64_t pos2 = 0;
    while (true) {
        const uint64_t cur = pos2;
        pos2 = s.find(',', cur);
        assumptions.emplace_back(parser::parse_expr(s.substr(cur, (pos2 == std::string::npos ? pos : pos2) - cur)));
        if (pos2 == std::string::npos)
            break;
        pos2++;
    }
    p_expr result(parser::parse_expr(s.substr(pos + 2)));
    return assumptions;
}

#endif //MATHLOGIC_COMMON_H
