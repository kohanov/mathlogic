#include "expr.h"
#include <vector>
#include <map>

#ifndef MATHLOGIC_PARSER_H
#define MATHLOGIC_PARSER_H

typedef std::map<std::string, std::string> name_map;
typedef std::vector<std::unique_ptr<expr>> expr_container;

struct parser {
    static int is_axiom(expr *expression, const expr_container &axioms);

    static bool match(expr *expression, expr *axiom, name_map &data, bool not_exact = true);

    expr *parse(const std::string expression);

private:

    expr *parse_impl();

    expr *parse_disj();

    expr *parse_conj();

    expr *parse_unary();

    std::string parse_name();

    int pos;
    std::string expression;
};

#endif //MATHLOGIC_PARSER_H
