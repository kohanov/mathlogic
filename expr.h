#include <string>
#include <memory>
#include <map>
#include <cassert>
#include <utility>
#include <vector>
#include <set>

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
    ANY,
    EXIST,
    PRED,
    EQ,
    SUM,
    MUL,
    INC,
};

struct expr {
    expr(EXPR_TYPE other_type) : type(other_type) {};

    static bool match(expr *expression, expr *axiom, bool variable_match = true);

    static uint32_t is_axiom(expr *expression, const expr_v &axioms);

    static std::pair<uint64_t, uint64_t>
    check_expr(const expr_v &axioms, const expr_v &assumptions,
               std::vector<std::pair<p_expr, uint32_t>> &good, const p_expr &expression);

    void free_vars(std::set<std::string> &res, std::map<std::string, uint32_t> &closed);

    static bool free_to_subst(const p_expr &expression, const p_expr &substitution, const p_expr &variable,
                              std::map<std::string, uint32_t> &close, std::map<std::string, std::string> &name_map);

    virtual ~expr() = default;

    virtual bool exec(const var_replace &data) = 0;

    EXPR_TYPE type;

    static name_map data;

    static std::wstring err_message;

    std::string str;
private:
    static bool match(expr *expression, expr *axiom, name_map &data, bool not_exact = true);
};

struct unary : expr {
    unary(EXPR_TYPE other_type) : expr(other_type) {};
};

struct var : unary {
    var(const std::string &value) : val(value), unary(EXPR_TYPE::VAR) { str = value; };

    var(const var &other) : var(other.str) {};

    bool exec(const var_replace &data) {
        auto it = data.find(val);
        assert(it != data.end());
        return it->second;
    }

    const std::string val;
};

struct neg : unary {
    neg(const p_expr &other) : unary(EXPR_TYPE::NEG), curr(other) {
        str = "!" + other->str;
    };

    neg(p_expr &&other) noexcept : unary(EXPR_TYPE::NEG), curr(std::move(other)) {
        str = "!" + curr->str;
    };

    bool exec(const var_replace &data) { return !curr->exec(data); }

    p_expr curr;
};

struct any : unary {
    any(const p_expr &variable, const p_expr &right) : var(variable), curr(right), unary(EXPR_TYPE::ANY) {
        assert(variable->type == EXPR_TYPE::VAR);
        str = "(@" + var->str + curr->str + ")";
    };

    any(p_expr &&variable, p_expr &&right) : var(std::move(variable)), curr(std::move(right)), unary(EXPR_TYPE::ANY) {
        assert(var->type == EXPR_TYPE::VAR);
        str = "(@" + var->str + curr->str + ")";
    };

    bool exec(const var_replace &data) {
        assert(false);
        return false;
    }

    p_expr curr;
    p_expr var;
};

struct exist : unary {
    exist(const p_expr &variable, const p_expr &right) : var(variable), curr(right), unary(EXPR_TYPE::EXIST) {
        assert(variable->type == EXPR_TYPE::VAR);
        str = "(?" + var->str + curr->str + ")";
    };

    exist(p_expr &&variable, p_expr &&right) : var(std::move(variable)), curr(std::move(right)),
                                               unary(EXPR_TYPE::EXIST) {
        assert(var->type == EXPR_TYPE::VAR);
        str = "(?" + var->str + curr->str + ")";
    };

    bool exec(const var_replace &data) {
        assert(false);
        return false;
    }

    p_expr curr;
    p_expr var;
};

struct pred : unary {
    pred(std::string name, expr_v args) : pred_name(std::move(name)),
                                          pred_args(std::move(args)),
                                          unary(EXPR_TYPE::PRED) {
        str = pred_name;
        if (!pred_args.empty()) {
            str.append("(");
            for (auto &pred_arg : pred_args) {
                str.append(pred_arg->str + ",");
            }
            str.back() = ')';
        }
    };

    bool exec(const var_replace &data) {
        //TODO
        return false;
    }

    const std::string pred_name;
    expr_v pred_args;
};

struct inc : unary {
    inc(const p_expr &val) : curr(val), unary(EXPR_TYPE::INC) {
        str = curr->str + "'";
    };

    inc(p_expr &&val) : curr(std::move(val)), unary(EXPR_TYPE::INC) {
        str = curr->str + "'";
    };

    bool exec(const var_replace &data) {
        assert(false);
        return false;
    }

    p_expr curr;
};

struct binary : expr {
    binary(const p_expr &left_value, const p_expr &right_value, EXPR_TYPE other_type, const std::string &op)
            : expr(other_type), left(left_value), right(right_value) {
        str = "(" + left->str + op + right->str + ")";
    };

    binary(p_expr &&left_value, p_expr &&right_value, EXPR_TYPE other_type, std::string &op) noexcept
            : expr(other_type), left(std::move(left_value)), right(std::move(right_value)) {
        str = "(" + left->str + op + right->str + ")";
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

struct eq : binary {
    eq(const p_expr &left_value, const p_expr &right_value) : binary(left_value, right_value,
                                                                     EXPR_TYPE::EQ, "=") {};

    eq(p_expr &&left_value, p_expr &&right_value) noexcept
            : binary(left_value, right_value, EXPR_TYPE::EQ, "=") {};

    bool exec(const var_replace &data) {
        assert(false);
        return false;
    }
};

struct sum : binary {
    sum(const p_expr &left_value, const p_expr &right_value) : binary(left_value, right_value,
                                                                      EXPR_TYPE::SUM, "+") {};

    sum(p_expr &&left_value, p_expr &&right_value) noexcept
            : binary(left_value, right_value, EXPR_TYPE::SUM, "+") {};

    bool exec(const var_replace &data) {
        assert(false);
        return false;
    }
};

struct mul : binary {
    mul(const p_expr &left_value, const p_expr &right_value) : binary(left_value, right_value,
                                                                      EXPR_TYPE::MUL, "*") {};

    mul(p_expr &&left_value, p_expr &&right_value) noexcept
            : binary(left_value, right_value, EXPR_TYPE::MUL, "*") {};

    bool exec(const var_replace &data) {
        assert(false);
        return false;
    }
};

std::wstring to_wstr(const std::string& s);

#endif //MATHLOGIC_EXPR_H
