#include "expr.h"
#include "parser.h"

#ifndef MATHLOGIC_PROOF_H
#define MATHLOGIC_PROOF_H

struct proof {
    expr_v assumptions;
    expr_v expressions;

    proof() {
        if (remove_assumption_theorem.empty()) {
            remove_assumption_theorem = parser::load_proof("rules/A_notA.txt");
            remove_assumption_theorem.push_back(parser::parse_expr("(A->B)->(!A->B)->(A|!A->B)"));
            remove_assumption_theorem.push_back(parser::parse_expr("(!A->B)->(A|!A->B)"));
            remove_assumption_theorem.push_back(parser::parse_expr("A|!A->B"));
            remove_assumption_theorem.push_back(parser::parse_expr("B"));
        }
    };

    proof(proof &&other) noexcept : proof() {
        assumptions = std::move(other.assumptions);
        expressions = std::move(other.expressions);
    };

    ~proof() = default;

    proof(const proof &other) = delete;

    proof operator=(const proof &other) = delete;

    proof operator=(const proof &&other) = delete;

    static void merge_proofs(proof &one, proof &two, expr_v &axioms, p_expr &var, p_expr &term);

    static bool generate_proof(proof &proof, const expr_v &assumptions, const p_expr &expression);

private:
    static expr_v remove_assumption_theorem;
};

#endif //MATHLOGIC_PROOF_H
