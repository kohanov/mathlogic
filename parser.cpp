#include "parser.h"
#include <assert.h>

int parser::is_axiom(expr *expression, const expr_container &axioms) {
    for (int i = 0; i < axioms.size(); i++) {
        name_map empty = {};
        if (match(expression, axioms[i].get(), empty))
            return i + 1;
    }
    return 0;
}

bool parser::match(expr *expression, expr *axiom, name_map &data, bool not_exact) {
    assert(expression != nullptr && axiom != nullptr);
    if (axiom->type == EXPR_TYPE::VAR && (not_exact || axiom->type == expression->type)) {
        var *temp = dynamic_cast<var *>(axiom);
        assert(temp != nullptr);
        auto it = data.find(axiom->str());
        if (it != data.end()) {
            return it->second == expression->str();
        } else {
            data.insert(std::make_pair(temp->str(), expression->str()));
            return true;
        }
    }
    if (axiom->type == expression->type) {
        if (axiom->type == EXPR_TYPE::NEG) {
            return match(dynamic_cast<neg *>(expression)->curr.get(), dynamic_cast<neg *>(axiom)->curr.get(), data,
                         not_exact);
        } else {
            bool ok = match(dynamic_cast<binary *>(expression)->left.get(), dynamic_cast<binary *>(axiom)->left.get(),
                            data, not_exact);
            if (ok)
                ok = match(dynamic_cast<binary *>(expression)->right.get(), dynamic_cast<binary *>(axiom)->right.get(),
                           data, not_exact);
            return ok;
        }
    }
    return false;
}

expr *parser::parse(const std::string exp) {
    expression = exp;
    pos = 0;
    return parse_impl();
}

expr *parser::parse_impl() {
    std::unique_ptr<expr> res(parse_disj());
    if (pos < expression.length() && expression[pos] == '-') {
        pos += 2;
        return new impl(res.release(), parse_impl());
    } else {
        return res.release();
    }
}

expr *parser::parse_disj() {
    std::unique_ptr<expr> res(parse_conj());
    while (pos < expression.length() && expression[pos] == '&') {
        pos++;
        res.reset(new disj(res.release(), parse_disj()));
    }
    return res.release();
}

expr *parser::parse_conj() {
    std::unique_ptr<expr> res(parse_unary());
    while (pos < expression.length() && expression[pos] == '|') {
        pos++;
        res.reset(new conj(res.release(), parse_conj()));
    }
    return res.release();
}

expr *parser::parse_unary() {
    if (expression[pos] == '!') {
        pos++;
        return new neg(parse_unary());
    } else if (pos < expression.length() && expression[pos] == '(') {
        pos++;
        std::unique_ptr<expr> temp(parse_impl());
        pos++;
        return temp.release();
    }
    return new var(parse_name());
}

std::string parser::parse_name() {
    int it = pos;
    while (it < expression.length()) {
        const char c = expression[it];
        if (isdigit(expression[it]) || isupper(expression[it]))
            it++;
        else
            break;
    }
    std::string result = expression.substr(pos, it - pos);
    pos = it;
    return result;
}