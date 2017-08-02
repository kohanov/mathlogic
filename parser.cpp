#include "parser.h"
#include <fstream>

uint32_t parser::pos = 0;
std::string parser::expression;

expr_v parser::load_proof(const std::string &filename) {
    expr_v result;
    std::ifstream in(filename);
    std::string s;
    while (std::getline(in, s)) {
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
        p_expr right(parse_disj());
        res.reset(new disj(res, right));
    }
    return res;
}

p_expr parser::parse_conj() {
    p_expr res(parse_unary());
    while (pos < expression.length() && expression[pos] == '&') {
        pos++;
        p_expr right(parse_conj());
        res.reset(new conj(res, right));
    }
    return res;
}

p_expr parser::parse_unary() {
    if (expression[pos] == '!') {
        pos++;
        p_expr res(parse_unary());
        return p_expr(new neg(res));
    }
    if (pos < expression.length() && expression[pos] == '(') {
        pos++;
        p_expr temp(parse_impl());
        pos++;
        return temp;
    }
    return p_expr(new var(parse_name()));
}

std::string parser::parse_name() {
    uint32_t it = pos;
    while (it < expression.length()) {
        const char c = expression[it];
        if (isdigit(c) == 0 && isupper(c) == 0)
            break;
        else
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
            if (temp->val == "A")
                return replace[0];
            if (temp->val == "B")
                return replace[1];
            if (temp->val == "C")
                return replace[2];
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
    }
}