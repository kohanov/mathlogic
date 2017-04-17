#include <string>
#include <memory>

#ifndef MATHLOGIC_EXPR_H
#define MATHLOGIC_EXPR_H

enum EXPR_TYPE {
    UNDEFINED,
    VAR,
    NEG,
    CONJ,
    DISJ,
    IMPL,
};

struct expr {
    expr(EXPR_TYPE other_type) : type(other_type) {};

    virtual std::string str() = 0;

    EXPR_TYPE type;
};

struct unary : expr {
    unary(EXPR_TYPE other_type) : expr(other_type) {};
};

struct var : unary {
    var(const std::string &value) : val(value), unary(EXPR_TYPE::VAR) {};

    std::string str() {
        return val;
    };
    const std::string val;
};

struct neg : unary {
    neg(expr *other) : unary(EXPR_TYPE::NEG) {
        curr.reset(other);
    };

    std::string str() {
        return std::string("!(").append(curr->str()).append(")");
    }

    std::shared_ptr<expr> curr;
};

struct binary : expr {
    binary(expr *left_value, expr *right_value, EXPR_TYPE other_type) : expr(other_type){
        left.reset(left_value);
        right.reset(right_value);
    };

    std::string str(const std::string &op) {
        return std::string("(").append(left->str()).append(op).append(right->str()).append(")");
    };

    std::shared_ptr<expr> left;
    std::shared_ptr<expr> right;
};

struct conj : binary {
    conj(expr *left_value, expr *right_value) : binary(left_value, right_value, EXPR_TYPE::CONJ) {};

    std::string str() {
        return binary::str("&");
    };
};

struct disj : binary {
    disj(expr *left_value, expr *right_value) : binary(left_value, right_value, EXPR_TYPE::DISJ) {};

    std::string str() {
        return binary::str("|");
    };
};

struct impl : binary {
    impl(expr *left_value, expr *right_value) : binary(left_value, right_value, EXPR_TYPE::IMPL) {};

    std::string str() {
        return binary::str("->");
    };
};

#endif //MATHLOGIC_EXPR_H
