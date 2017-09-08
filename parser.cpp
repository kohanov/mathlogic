#include "parser.h"
#include <fstream>

uint32_t parser::pos = 0;
std::string parser::expression;
bool parser::predicate = false;
bool parser::balance = true;

expr_v parser::load_proof(const std::string &filename, bool to_lower) {
    expr_v result;
    std::ifstream in(filename);
    std::string s;
    while (std::getline(in, s)) {
        if (to_lower) {
            for (char &c : s) {
                if (isalpha(c)) {
                    c = tolower(c);
                }
            }
        }
        result.push_back(p_expr(parser::parse_expr(s)));
    }
    in.close();
    return result;
}

p_expr parser::parse_expr(const std::string &exp) {
    expression = exp;
    pos = 0;
    return parse_impl();
}

p_expr parser::parse_impl() {
    p_expr res(parse_disj());
    if (pos < expression.length() && expression[pos] == '-') {
        pos += 2;
        p_expr right(parse_impl());
        return p_expr(new impl(res, right));
    }
    return res;
}

p_expr parser::parse_disj() {
    p_expr res(parse_conj());
    while (pos < expression.length() && expression[pos] == '|') {
        pos++;
        p_expr right(parse_conj());
        res.reset(new disj(res, right));
    }
    return res;
}

p_expr parser::parse_conj() {
    p_expr res(parse_unary());
    while (pos < expression.length() && expression[pos] == '&') {
        pos++;
        p_expr right(parse_unary());
        res.reset(new conj(res, right));
    }
    return res;
}

p_expr parser::parse_unary() {
    assert(pos < expression.length());
    char cur_char = expression[pos];
    if (cur_char == '!') {
        pos++;
        p_expr res(parse_unary());
        return p_expr(new neg(res));
    }
    if (predicate) {
        if (cur_char == '@' || cur_char == '?') {
            pos++;
            p_expr quant_var(new var(parse_name()));
            p_expr right_expr(parse_unary());
            if (cur_char == '@')
                return p_expr(new any(quant_var, right_expr));
            else
                return p_expr(new exist(quant_var, right_expr));
        }

        p_expr pred(parse_pred());
        if (pred != nullptr && balance)
            return pred;
        balance = true;
    }
    if (cur_char == '(') {
        pos++;
        p_expr temp(parse_impl());
        pos++;
        return temp;
    }
    return p_expr(new var(predicate ? parse_name() : parse_pred_name()));
}

p_expr parser::parse_pred() {
    std::string pred_name = parse_pred_name();
    if (!pred_name.empty())
        return p_expr(new pred(pred_name, parse_term_list()));

    uint32_t it = pos;
    auto term = parse_term();
    uint32_t it2 = pos;
    if (pos >= expression.length() || expression[it2] != '=') {
        pos = it;
        return nullptr;
    }
    pos++;
    return p_expr(new eq(term, parse_term()));
}

expr_v parser::parse_term_list() {
    expr_v result;
    if (pos >= expression.length() || expression[pos] != '(')
        return result;
    pos++;
    result.push_back(parse_term());
    while (pos < expression.length() && expression[pos] != ')') {
        pos++;
        result.push_back(parse_term());
    }
    pos++;
    return result;
}

p_expr parser::parse_term() {
    p_expr res(parse_sum());
    while (pos < expression.length() && expression[pos] == '+') {
        pos++;
        p_expr right(parse_sum());
        res.reset(new sum(res, right));
    }
    return res;
}

p_expr parser::parse_sum() {
    p_expr res(parse_mul());
    while (pos < expression.length() && expression[pos] == '*') {
        pos++;
        p_expr right(parse_mul());
        res.reset(new mul(res, right));
    }
    return res;
}

p_expr parser::parse_mul() {
    p_expr res;
    if (pos < expression.length() && expression[pos] == '(') {
        pos++;
        res = parse_term();
        if (expression[pos] != ')')
            balance = false;
        pos++;
        return parse_inc(res);
    }
    const std::string var_name(parse_name());
    if (pos < expression.length() && expression[pos] == '(') {
        res.reset(new pred(var_name, parse_term_list()));
    } else {
        res.reset(new var(var_name));
    }
    return parse_inc(res);
}

p_expr parser::parse_inc(p_expr var) {
    while (pos < expression.length() && expression[pos] == '\'') {
        pos++;
        var.reset(new inc(var));
    }
    assert(var != nullptr);
    return var;
}

std::string parser::parse_name() {
    uint32_t it = pos;
    while (it < expression.length()) {
        const char c = expression[it];
        if (isdigit(c) == 0 && islower(c) == 0)
            break;
        it++;
    }
    std::string result = expression.substr(pos, it - pos);
    pos = it;
    return result;
}

std::string parser::parse_pred_name() {
    uint32_t it = pos;
    while (it < expression.length()) {
        const char c = expression[it];
        if (isdigit(c) != 0 || isupper(c) == 0)
            break;
        it++;
    }
    std::string result = expression.substr(pos, it - pos);
    pos = it;
    return result;
}

p_expr parser::change(const p_expr &base, const expr_v &replace) {
    switch (base->type) {
        case EXPR_TYPE::VAR: {
            auto *temp = dynamic_cast<var *>(base.get());
            std::vector<std::string> replace_vars = {"A", "B", "C", "X"};
            if (predicate)
                replace_vars = {"a", "b", "c", "x"};
            for (uint32_t i = 0; i < replace_vars.size(); i++)
                if (temp->val == replace_vars[i])
                    return replace[i];
            assert(false);
        }
        case EXPR_TYPE::NEG: {
            auto *temp = dynamic_cast<neg *>(base.get());
            return p_expr(new neg(change(temp->curr, replace)));
        }
        case EXPR_TYPE::DISJ: {
            auto *temp = dynamic_cast<disj *>(base.get());
            return p_expr(new disj(change(temp->left, replace), change(temp->right, replace)));
        }
        case EXPR_TYPE::CONJ: {
            auto *temp = dynamic_cast<conj *>(base.get());
            return p_expr(new conj(change(temp->left, replace), change(temp->right, replace)));
        }
        case EXPR_TYPE::IMPL: {
            auto *temp = dynamic_cast<impl *>(base.get());
            return p_expr(new impl(change(temp->left, replace), change(temp->right, replace)));
        }
        case EXPR_TYPE::ANY: {
            auto *temp = dynamic_cast<any *>(base.get());
            return p_expr(new any(change(temp->var, replace), change(temp->curr, replace)));
        }
        case EXPR_TYPE::EXIST: {
            auto *temp = dynamic_cast<exist *>(base.get());
            return p_expr(new exist(change(temp->var, replace), change(temp->curr, replace)));
        }
    }
}