#include "expr.h"

#ifndef MATHLOGIC_PARSER_H
#define MATHLOGIC_PARSER_H

struct parser {
    static expr_v load_proof(const std::string &filename);

    static p_expr parse_expr(const std::string &expression);

    static p_expr change(const p_expr &base, const expr_v &replace);

private:

    static p_expr parse_impl();

    static p_expr parse_disj();

    static p_expr parse_conj();

    static p_expr parse_unary();

    static std::string parse_name();

    static uint32_t pos;
    static std::string expression;
};

#endif //MATHLOGIC_PARSER_H
