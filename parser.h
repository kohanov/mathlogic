#include "expr.h"

#ifndef MATHLOGIC_PARSER_H
#define MATHLOGIC_PARSER_H

struct parser {
    static expr_v load_proof(const std::string &filename, bool to_lower = false);

    static p_expr parse_expr(const std::string &expression);

    static p_expr change(const p_expr &base, const expr_v &replace);

    static bool predicate;
private:
    static p_expr parse_impl();

    static p_expr parse_disj();

    static p_expr parse_conj();

    static p_expr parse_unary();

    static p_expr parse_pred();

    static expr_v parse_term_list();

    static p_expr parse_term();

    static p_expr parse_sum();

    static p_expr parse_mul();

    static p_expr parse_inc(p_expr var);

    static std::string parse_name();

    static std::string parse_pred_name();

    static uint32_t pos;
    static std::string expression;
    static bool balance;
};

#endif //MATHLOGIC_PARSER_H
