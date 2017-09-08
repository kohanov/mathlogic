#include "expr.h"

name_map expr::data = {};
std::wstring expr::err_message;

bool expr::match(expr *expression, expr *axiom, bool variable_match) {
    data.clear();
    return match(expression, axiom, data, variable_match);
}

bool expr::match(expr *expression, expr *axiom, name_map &data, bool variable_match) {
    assert(expression != nullptr && axiom != nullptr);
    if (axiom->type == EXPR_TYPE::VAR && (variable_match || axiom->type == expression->type)) {
        auto *variable = dynamic_cast<var *>(axiom);
        assert(variable != nullptr);
        auto it = data.find(axiom->str);
        if (it != data.end()) {
            return it->second == expression->str;
        }
        data.insert(std::make_pair(variable->str, expression->str));
        return variable_match || variable->str == expression->str;
    }
    if (axiom->type == expression->type) {
        if (axiom->type == EXPR_TYPE::NEG) {
            return match(dynamic_cast<neg *>(expression)->curr.get(), dynamic_cast<neg *>(axiom)->curr.get(),
                         expr::data, variable_match);
        }
        if (axiom->type == EXPR_TYPE::INC) {
            return match(dynamic_cast<inc *>(expression)->curr.get(), dynamic_cast<inc *>(axiom)->curr.get(),
                         expr::data, variable_match);
        }
        if (axiom->type == EXPR_TYPE::ANY) {
            auto any_expr = dynamic_cast<any *>(expression);
            auto any_axiom = dynamic_cast<any *>(axiom);
            if (match(any_expr->var.get(), any_axiom->var.get(), expr::data, variable_match))
                return match(any_expr->curr.get(), any_axiom->curr.get(), expr::data, variable_match);
            return false;
        }
        if (axiom->type == EXPR_TYPE::EXIST) {
            auto exist_expr = dynamic_cast<exist *>(expression);
            auto exist_axiom = dynamic_cast<exist *>(axiom);
            if (match(exist_expr->var.get(), exist_axiom->var.get(), expr::data, variable_match))
                return match(exist_expr->curr.get(), exist_axiom->curr.get(), expr::data, variable_match);
            return false;
        }
        if (axiom->type == EXPR_TYPE::PRED) {
            auto pred_expr = dynamic_cast<pred *>(expression);
            auto pred_axiom = dynamic_cast<pred *>(axiom);
            if (pred_expr->pred_args.size() != pred_axiom->pred_args.size())
                return false;
            for (uint32_t i = 0; i < pred_expr->pred_args.size(); i++) {
                if (!match(pred_expr->pred_args[i].get(), pred_axiom->pred_args[i].get(), expr::data, variable_match))
                    return false;
            }
            auto it = data.find(pred_axiom->pred_name);
            if (it != data.end()) {
                return it->second == pred_expr->pred_name;
            }
            data.insert(std::make_pair(pred_axiom->pred_name, pred_expr->pred_name));
            return variable_match || pred_axiom->pred_name == pred_expr->pred_name;
        }
        bool left_match = match(dynamic_cast<binary *>(expression)->left.get(),
                                dynamic_cast<binary *>(axiom)->left.get(),
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
    for (uint32_t i = 0; i < assumptions.size(); i++) {
        if (match(expression.get(), assumptions[i].get(), false)) {
            return {0, i + 1};
        }
    }
    for (int64_t j = good.size() - 1; j > -1; j--) {
        if (good[j].first->type == EXPR_TYPE::IMPL) {
            for (int64_t i = good.size() - 1; i > -1; i--) {
                if (i == j)
                    continue;
                auto *temp = dynamic_cast<impl *>(good[j].first.get());
                if (match(good[i].first.get(), temp->left.get(), false)) {
                    if (match(expression.get(), temp->right.get(), false)) {
                        return {good[i].second, good[j].second};
                    }
                }
            }
        }
    }
    return {0, 0};
}

void expr::free_vars(std::set<std::string> &res, std::map<std::string, uint32_t> &closed) {
    if (type == EXPR_TYPE::VAR) {
        auto var_name = dynamic_cast<var *>(this)->val;
        if (closed.find(var_name) == closed.end())
            res.insert(var_name);
    } else if (type == EXPR_TYPE::NEG)
        dynamic_cast<neg *>(this)->curr->free_vars(res, closed);
    else if (type == EXPR_TYPE::INC)
        dynamic_cast<inc *>(this)->curr->free_vars(res, closed);
    else if (type == EXPR_TYPE::ANY || type == EXPR_TYPE::EXIST) {
        std::string var_name;
        p_expr temp;
        if (type == EXPR_TYPE::ANY) {
            auto any_expr = dynamic_cast<any *>(this);
            var_name = any_expr->var->str;
            temp = any_expr->curr;
        } else {
            auto exist_expr = dynamic_cast<exist *>(this);
            var_name = exist_expr->var->str;
            temp = exist_expr->curr;
        }
        closed[var_name]++;
        temp->free_vars(res, closed);
        if (closed[var_name] == 1)
            closed.erase(var_name);
        else
            closed[var_name]--;
    } else if (type == EXPR_TYPE::PRED) {
        for (auto &e : dynamic_cast<pred *>(this)->pred_args)
            e->free_vars(res, closed);
    } else {
        auto b = dynamic_cast<binary *>(this);
        assert(b != nullptr);
        b->left->free_vars(res, closed);
        b->right->free_vars(res, closed);
    }
}

bool expr::free_to_subst(const p_expr &expression, const p_expr &substitution, const p_expr &variable,
                         std::map<std::string, uint32_t> &closed, name_map &name_map) {
    err_message.clear();
    if (expression->type == EXPR_TYPE::VAR) {
        if (!expr::match(expression.get(), variable.get(), false)) {
            auto result = expr::match(expression.get(), substitution.get(), false);
            if (!result)
                err_message = L"Неправильное применение правила вывода";
            return result;
        }
        if (closed[expression->str] != 0) {
            auto result = expr::match(expression.get(), substitution.get(), false);
            if (!result)
                err_message = L"Неправильное применение правила вывода";
            return result;
        }
        if (name_map.find(expression->str) != name_map.end()) {
            auto result = name_map[expression->str] == substitution->str;
            if (!result)
                err_message = L"Неправильное применение правила вывода";
            return result;
        }
        std::set<std::string> free_variables;
        std::map<std::string, uint32_t> closed2;
        substitution->free_vars(free_variables, closed2);
        for (auto &free_variable: free_variables)
            if (closed[free_variable] != 0) {
                err_message = L"Терм " + to_wstr(substitution->str) + L" подставлен вместо замкнутой переменной " +
                              to_wstr(expression->str);
                return false;
            }
        name_map[expression->str] = substitution->str;
        return true;
    }
    if (expression->type != substitution->type) {
        return false;
    }
    switch (expression->type) {
        case EXPR_TYPE::ANY: {
            auto any_expr = dynamic_cast<any *>(expression.get());
            closed[any_expr->var->str]++;
            auto is_free = free_to_subst(any_expr->curr, dynamic_cast<any *>(substitution.get())->curr, variable,
                                         closed,
                                         name_map);
            if (closed[any_expr->var->str] == 1)
                closed.erase(any_expr->var->str);
            else
                closed[any_expr->var->str]--;
            return is_free;
        }
        case EXPR_TYPE::EXIST: {
            auto exist_expr = dynamic_cast<exist *>(expression.get());
            closed[exist_expr->var->str]++;
            auto is_free = free_to_subst(exist_expr->curr, dynamic_cast<exist *>(substitution.get())->curr, variable,
                                         closed,
                                         name_map);
            if (closed[exist_expr->var->str] == 1)
                closed.erase(exist_expr->var->str);
            else
                closed[exist_expr->var->str]--;
            return is_free;
        }
        case EXPR_TYPE::PRED: {
            auto pred_quant = dynamic_cast<pred *>(expression.get());
            auto pred_expr = dynamic_cast<pred *>(substitution.get());
            if (pred_quant->pred_args.size() != pred_expr->pred_args.size()) {
                err_message = L"Неправильное применение правила вывода";
                return false;
            }
            for (uint32_t i = 0; i < pred_quant->pred_args.size(); i++)
                if (!free_to_subst(pred_quant->pred_args[i], pred_expr->pred_args[i], variable, closed, name_map))
                    return false;
            return true;
        }
        case EXPR_TYPE::INC:
            return free_to_subst(dynamic_cast<inc *>(expression.get())->curr,
                                 dynamic_cast<inc *>(substitution.get())->curr,
                                 variable, closed, name_map);
        case EXPR_TYPE::NEG:
            return free_to_subst(dynamic_cast<neg *>(expression.get())->curr,
                                 dynamic_cast<neg *>(substitution.get())->curr,
                                 variable, closed, name_map);
        default: {
            auto quant_expr = dynamic_cast<binary *>(expression.get());
            auto expr_expr = dynamic_cast<binary *>(substitution.get());
            if (free_to_subst(quant_expr->left, expr_expr->left, variable, closed, name_map))
                return free_to_subst(quant_expr->right, expr_expr->right, variable, closed, name_map);
        }
    }
}

std::wstring to_wstr(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}