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

    std::unique_ptr<expr> parse(const std::string expression);

private:

    std::unique_ptr<expr> parse_impl();

    std::unique_ptr<expr> parse_disj();

    std::unique_ptr<expr> parse_conj();

    std::unique_ptr<expr> parse_unary();

    std::string parse_name();

    int pos;
    std::string expression;
};

#endif //MATHLOGIC_PARSER_H
