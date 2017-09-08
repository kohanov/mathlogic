#include "proof.h"
#include <iostream>

expr_v proof::remove_assumption_theorem;

void proof::merge_proofs(proof &one, proof &two, expr_v &axioms, p_expr &var, p_expr &term) {
    for (auto &expr : two.expressions) {
        one.expressions.push_back(std::move(expr));
    }
    for (auto &rule : remove_assumption_theorem) {
        one.expressions.push_back(parser::change(rule, {var, term}));
    }
}

bool proof::generate_proof(proof &proof, const expr_v &assumptions, const p_expr &expression) {
    switch (expression->type) {
        case EXPR_TYPE::VAR: {
            auto *variable = dynamic_cast<var *>(expression.get());
            p_expr temp(new var(variable->str));
            for (const auto &assumption : assumptions) {
                if (assumption->str == variable->str) {
                    proof.expressions.emplace_back(new var(variable->str));
                    return true;
                }
            }
            proof.expressions.emplace_back(new neg(p_expr(new var(variable->str))));
            return false;
        }
        case EXPR_TYPE::NEG: {
            auto *negate = dynamic_cast<neg *>(expression.get());
            bool present = generate_proof(proof, assumptions, negate->curr);
            if (present) {
                for (auto &rule : parser::load_proof("rules/A_notnotA.txt")) {
                    proof.expressions.push_back(parser::change(rule, {negate->curr}));
                }
            }
            return !present;
        }
        default: {
            std::string proof_name;
            if (expression->type == EXPR_TYPE::CONJ)
                proof_name.append("and");
            else if (expression->type == EXPR_TYPE::DISJ)
                proof_name.append("or");
            else if (expression->type == EXPR_TYPE::IMPL)
                proof_name.append("impl");
            else {
                std::cout << "Error" << std::endl;
                return false;
            }
            auto *binary_expr = dynamic_cast<binary *>(expression.get());
            bool presentA = generate_proof(proof, assumptions, binary_expr->left);
            bool presentB = generate_proof(proof, assumptions, binary_expr->right);
            unsigned int rule_number = presentA ? 2 : 4;
            if (presentB)
                rule_number--;
            proof_name.append(std::to_string(rule_number) + ".txt");
            for (auto &rule : parser::load_proof("rules/" + proof_name)) {
                proof.expressions.push_back(parser::change(rule, {binary_expr->left, binary_expr->right}));
            }
            return expr::match(proof.expressions.back().get(), expression.get(), false);
        }
    }
}
