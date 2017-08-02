#include <string>
#include <memory>
#include <map>
#include <cassert>
#include <vector>

#ifndef MATHLOGIC_EXPR_H
#define MATHLOGIC_EXPR_H

struct expr;

typedef std::shared_ptr<expr> p_expr;
typedef std::map<std::string, std::string> name_map;
typedef std::vector<p_expr> expr_v;
typedef std::map<std::string, bool> var_replace;

enum EXPR_TYPE {
    VAR,
    NEG,
    CONJ,
    DISJ,
    IMPL,
};

struct expr {
    expr(EXPR_TYPE other_type) : type(other_type) {};

    static bool match(expr *expression, expr *axiom, bool variable_match = true);

    static uint32_t is_axiom(expr *expression, const expr_v &axioms);

    static std::pair<uint64_t, uint64_t>
    check_expr(const expr_v &axioms, const expr_v &assumptions,
               std::vector<std::pair<p_expr, uint32_t>> &good, const p_expr &expression);

    virtual ~expr() = default;

    virtual bool exec(const var_replace &data) = 0;

    EXPR_TYPE type;

    static name_map data;

    std::string term;
private:
    static bool match(expr *expression, expr *axiom, name_map &data, bool not_exact = true);
};

struct unary : expr {
    unary(EXPR_TYPE other_type) : expr(other_type) {};
};

struct var : unary {
    var(const std::string &value) : val(value), unary(EXPR_TYPE::VAR) { term = value; };

    var(const var &other) : var(other.term) {};

    bool exec(const var_replace &data) {
        auto it = data.find(val);
        assert(it != data.end());
        return it->second;
    }

    const std::string val;
};

struct neg : unary {
    neg(const p_expr &other) : unary(EXPR_TYPE::NEG), curr(other) {
        term = "!" + other->term;
    };

    neg(p_expr &&other) noexcept : unary(EXPR_TYPE::NEG), curr(std::move(other)) {
        term = "!" + curr->term;
    };

    bool exec(const var_replace &data) { return !curr->exec(data); }

    p_expr curr;
};

struct binary : expr {
    binary(const p_expr &left_value, const p_expr &right_value, EXPR_TYPE other_type, const std::string &op)
            : expr(other_type), left(left_value), right(right_value) {
        term = "(" + left->term + op + right->term + ")";
    };

    binary(p_expr &&left_value, p_expr &&right_value, EXPR_TYPE other_type, std::string &op) noexcept
            : expr(other_type), left(std::move(left_value)), right(std::move(right_value)) {
        term = "(" + left->term + op + right->term + ")";
    };

    p_expr left;
    p_expr right;
};

struct conj : binary {
    conj(const p_expr &left_value, const p_expr &right_value)
            : binary(left_value, right_value, EXPR_TYPE::CONJ, "&") {};

    conj(p_expr &&left_value, p_expr &&right_value) noexcept
            : binary(left_value, right_value, EXPR_TYPE::CONJ, "&") {};

    bool exec(const var_replace &data) { return left->exec(data) && right->exec(data); }
};

struct disj : binary {
    disj(const p_expr &left_value, const p_expr &right_value)
            : binary(left_value, right_value, EXPR_TYPE::DISJ, "|") {};

    disj(p_expr &&left_value, p_expr &&right_value) noexcept
            : binary(left_value, right_value, EXPR_TYPE::DISJ, "|") {};

    bool exec(const var_replace &data) { return left->exec(data) || right->exec(data); }
};

struct impl : binary {
    impl(const p_expr &left_value, const p_expr &right_value) : binary(left_value, right_value,
                                                                       EXPR_TYPE::IMPL, "->") {};

    impl(p_expr &&left_value, p_expr &&right_value) noexcept
            : binary(left_value, right_value, EXPR_TYPE::IMPL, "->") {};

    bool exec(const var_replace &data) { return !left->exec(data) || right->exec(data); }
};

#endif //MATHLOGIC_EXPR_H
