#include "expr.h"

name_map expr::data = {};

bool expr::match(expr *expression, expr *axiom, bool variable_match) {
    data.clear();
    return match(expression, axiom, data, variable_match);
}

bool expr::match(expr *expression, expr *axiom, name_map &data, bool variable_match) {
    assert(expression != nullptr && axiom != nullptr);
    if (axiom->type == EXPR_TYPE::VAR && (variable_match || axiom->type == expression->type)) {
        auto *variable = dynamic_cast<var *>(axiom);
        assert(variable != nullptr);
        auto it = data.find(axiom->term);
        if (it != data.end()) {
            return it->second == expression->term;
        }
        data.insert(std::make_pair(variable->term, expression->term));
        return variable_match || variable->term == expression->term;
    }
    if (axiom->type == expression->type) {
        if (axiom->type == EXPR_TYPE::NEG) {
            return match(dynamic_cast<neg *>(expression)->curr.get(), dynamic_cast<neg *>(axiom)->curr.get(),
                         expr::data, variable_match);
        }
        bool left_match = match(dynamic_cast<binary *>(expression)->left.get(), dynamic_cast<binary *>(axiom)->left.get(),
                        expr::data, variable_match);
        if (left_match)
            return match(dynamic_cast<binary *>(expression)->right.get(), dynamic_cast<binary *>(axiom)->right.get(),
                       expr::data, variable_match);
        return false;
    }
    return false;
}

uint32_t expr::is_axiom(expr *expression, const expr_v &axioms) {
    for (uint32_t i = 0; i < axioms.size(); i++) {
        if (match(expression, axioms[i].get()))
            return i + 1;
    }
    return 0;
}

std::pair<uint64_t, uint64_t>
expr::check_expr(const expr_v &axioms, const expr_v &assumptions,
                 std::vector<std::pair<p_expr, uint32_t >> &good, const p_expr &expression) {
    auto matching = is_axiom(expression.get(), axioms);
    if (matching != 0) {
        return {matching, 0};
    }
    uint32_t assumption_number = 0;
    for (uint32_t i = 0; i < assumptions.size(); i++) {
        if (match(expression.get(), assumptions[i].get(), false)) {
            assumption_number = i + 1;
            break;
        }
    }
    if (assumption_number != 0) {
        return {0, assumption_number};
    }
    uint64_t iMP = 0, jMP = 0;
    for (int64_t j = good.size() - 1; j > -1; j--) {
        if (good[j].first->type == EXPR_TYPE::IMPL) {
            for (int64_t i = good.size() - 1; i > -1; i--) {
                if (i == j)
                    continue;
                auto *temp = dynamic_cast<impl *>(good[j].first.get());
                if (match(good[i].first.get(), temp->left.get(), false)) {
                    if (match(expression.get(), temp->right.get(), false)) {
                        iMP = good[i].second;
                        jMP = good[j].second;
                        break;
                    }
                }
            }
            if (iMP > 0)
                break;
        }
    }
    if (iMP > 0)
        return {iMP, jMP};
    return {0, 0};
}