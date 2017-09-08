#include <codecvt>
#include <iostream>
#include <fstream>

#include "common.h"

int main() {
    std::cout << R"(Usage: "input_file" "output_file")" << std::endl;
    std::string input_file, output_file;
    std::cin >> input_file >> output_file;
    std::ifstream in(input_file);
    std::wofstream out(output_file);
    out.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>()));
    parser::predicate = true;
    expr_v axioms = parser::load_proof("rules/axioms.txt", true);
    for (auto &rule : parser::load_proof("rules/predicate_axioms.txt")) {
        axioms.emplace_back(rule);
    }
    expr_v assumptions;
    std::string s;
    std::getline(in, s);
    const uint64_t pos = s.find("|-");
    if (pos == std::string::npos || pos + 2 == s.length()) {
        out << L"Ввод не соответствует грамматике" << std::endl;
        in.close();
        out.close();
        return 1;
    }
    assumptions = parse_header(s);
    std::set<std::string> last_assumption_free_vars;
    if (!assumptions.empty()) {
        std::map<std::string, uint32_t> temp;
        assumptions.back()->free_vars(last_assumption_free_vars, temp);
    }
    p_expr B(parser::parse_expr(s.substr(pos + 2)));
    std::vector<std::pair<uint32_t, uint32_t>> info;
    std::vector<p_expr> expressions;
    std::vector<std::vector<int64_t>> mp_index;
    uint32_t line = 0;
    std::wstring error_message;
    while (std::getline(in, s) && error_message.empty()) {
        p_expr e(parser::parse_expr(s));
        line++;
        if (line % 1000 == 0)
            std::cout << line << std::endl;
        bool match = false;
        uint32_t axiom_number = expr::is_axiom(e.get(), axioms);
        if (axiom_number > 0) {
            match = true;
            info.emplace_back(0, axiom_number);
        }
        if (!match) {
            for (size_t i = 0; i < assumptions.size(); i++) {
                if (expr::match(e.get(), assumptions[i].get(), false)) {
                    match = true;
                    info.emplace_back(1, i);
                    break;
                }
            }
        }
        if (!match) { // M.P.
            for (int64_t j = expressions.size() - 1; j > 0; j--) {
                if (!mp_index[j].empty()) {
                    auto impl_expr = dynamic_cast<impl *>(expressions[j].get());
                    if (expr::match(e.get(), impl_expr->right.get(), false)) {
                        info.emplace_back(2, j);
                        match = true;
                        break;
                    }
                }
            }
        }
        if (!match && e->type == EXPR_TYPE::IMPL) {
            std::set<std::string> free_variables;
            std::map<std::string, uint32_t> closed;
            auto impl_expr = dynamic_cast<impl *>(e.get());
            p_expr to_search = nullptr, variable = nullptr;
            if (impl_expr->left->type == EXPR_TYPE::EXIST) {
                auto left_expr = dynamic_cast<exist *>(impl_expr->left.get());
                to_search.reset(new impl(left_expr->curr, impl_expr->right));
                impl_expr->right->free_vars(free_variables, closed);
                variable = left_expr->var;
            } else if (impl_expr->right->type == EXPR_TYPE::ANY) {
                auto right_expr = dynamic_cast<any *>(impl_expr->right.get());
                to_search.reset(new impl(impl_expr->left, right_expr->curr));
                impl_expr->left->free_vars(free_variables, closed);
                variable = right_expr->var;
            }
            if (to_search != nullptr) {
                for (int64_t j = expressions.size() - 1; j >= 0; j--) {
                    if (expressions[j]->type == EXPR_TYPE::IMPL) {
                        if (expr::match(expressions[j].get(), to_search.get(), false)) {
                            if (free_variables.find(variable->str) != free_variables.end()) {
                                error_message =
                                        L"Переменная " + to_wstr(variable->str) + L" входит свободно в формулу " +
                                        to_wstr(impl_expr->str);
                            } else if (last_assumption_free_vars.find(variable->str) !=
                                       last_assumption_free_vars.end()) {
                                error_message = L"Используется правило с квантором по переменной \"" +
                                                to_wstr(variable->str) + L"\", входящей свободно в допущение " +
                                                to_wstr(assumptions.back()->str);
                            } else {
                                match = true;
                                info.emplace_back(impl_expr->right->type == EXPR_TYPE::ANY ? 3 : 4, j);
                            }
                            break;
                        }
                    }
                }
            }
            if (!match && impl_expr->left->type == EXPR_TYPE::CONJ) {
                auto disj_expr = dynamic_cast<conj *>(impl_expr->left.get());
                if (disj_expr->right->type == EXPR_TYPE::ANY) {
                    auto any_expr = dynamic_cast<any *>(disj_expr->right.get());
                    auto var_expr = any_expr->var;
                    if (any_expr->curr->type == EXPR_TYPE::IMPL) {
                        auto implication = dynamic_cast<impl *>(any_expr->curr.get());
                        if (expr::match(implication->left.get(), impl_expr->right.get(), true)) {
                            if (expr::match(disj_expr->left.get(), implication->left.get())) {
                                if (expr::data[var_expr->str] == "0") {
                                    if (expr::match(implication->right.get(), implication->left.get())) {
                                        if (p_expr(new inc(var_expr))->str == expr::data[var_expr->str]) {
                                            // check free variables
                                            free_variables.clear();
                                            closed.clear();
                                            impl_expr->right->free_vars(free_variables, closed);
                                            if (free_variables.find(var_expr->str) != free_variables.end()) {
                                                match = true;
                                                info.emplace_back(0, 0);
                                            } else {
                                                error_message = L"Переменная " + to_wstr(var_expr->str) +
                                                                L" входит свободно в формулу " +
                                                                to_wstr(impl_expr->right->str);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (!match) {
                auto implication = dynamic_cast<impl *>(e.get());
                closed.clear();
                std::map<std::string, std::string> name_map;
                bool axioms_match = false, is_any_axiom = false;
                if (implication->left->type == EXPR_TYPE::ANY) {
                    auto any_expr = dynamic_cast<any *>(implication->left.get());
                    if (implication->right->type == any_expr->curr->type) {
                        is_any_axiom = true;
                        axioms_match = expr::free_to_subst(any_expr->curr, implication->right, any_expr->var,
                                                           closed,
                                                           name_map);
                        if (!axioms_match) {
                            error_message = expr::err_message;
                        }
                    }
                }
                if (!is_any_axiom && implication->right->type == EXPR_TYPE::EXIST) {
                    auto ex_expr = dynamic_cast<exist *>(implication->right.get());
                    if (implication->left->type, ex_expr->curr->type) {
                        axioms_match = expr::free_to_subst(ex_expr->curr, implication->left, ex_expr->var, closed,
                                                           name_map);
                        if (!axioms_match)
                            error_message = expr::err_message;
                    }
                }
                if (axioms_match) {
                    match = true;
                    info.emplace_back(0, 0);
                }
            }
        }
        if (match) {
            mp_index.emplace_back();
            if (e->type == EXPR_TYPE::IMPL) {
                auto impl_expr = dynamic_cast<impl *>(e.get());
                for (int64_t i = expressions.size() - 1; i > -1; i--) {
                    if (expr::match(expressions[i].get(), impl_expr->left.get(), false)) {
                        mp_index.back().push_back(i);
                    }
                }
            }
            for (int64_t i = expressions.size() - 1; i > -1; i--) {
                if (expressions[i]->type == EXPR_TYPE::IMPL) {
                    auto *impl_expr = dynamic_cast<impl *>(expressions[i].get());
                    if (expr::match(e.get(), impl_expr->left.get(), false)) {
                        mp_index[i].push_back(expressions.size());
                    }
                }
            }
            expressions.emplace_back(e);
        } else {
            error_message = L"Вывод некорректен начиная с формулы номер " + std::to_wstring(line) +
                            (error_message.empty() ? L"" : L": " + error_message);
            break;
        }
    }

    if (!error_message.empty()) {
        std::wcout << error_message << std::endl;
        out << error_message.c_str() << std::endl;
    } else if (assumptions.empty()) {
        std::cout << "OK" << std::endl;
        out << L"|-" << B->str.c_str() << std::endl;
        for (auto &expression : expressions) {
            out << expression->str.c_str() << std::endl;
        }
    } else {
        std::cout << "OK" << std::endl;
        for (uint32_t i = 0; i < assumptions.size() - 1; i++) {
            out << assumptions[i]->str.c_str();
            if (i != assumptions.size() - 2)
                out << ",";
        }
        p_expr AimplB(new impl(assumptions.back(), B));
        out << "|-" << AimplB->str.c_str() << std::endl;
        for (uint32_t i = 0; i < expressions.size(); i++) {
            switch (info[i].first) {
                case 1: {
                    // assumption
                    if (info[i].second == assumptions.size() - 1) {
                        for (auto &rule : parser::load_proof("rules/AimplA.txt", true)) {
                            out << parser::change(rule, {expressions[i]})->str.c_str() << std::endl;
                        }
                        break;
                    }
                }
                case 0: {
                    // axiom
                    out << expressions[i]->str.c_str() << std::endl;
                    p_expr temp(new impl(assumptions.back(), expressions[i]));
                    out << p_expr(new impl(expressions[i], temp))->str.c_str() << std::endl;
                    out << temp->str.c_str() << std::endl;
                    break;
                }
                case 2: {
                    // MP
                    expr_v mp;
                    mp.push_back(parser::parse_expr("(a->b)->((a->(b->c))->(a->c))"));
                    mp.push_back(parser::parse_expr("((a->(b->c))->(a->c))"));
                    mp.push_back(parser::parse_expr("a->c"));
                    for (auto &exp : mp) {
                        int64_t temp_pos = mp_index[info[i].second].front();
                        out << parser::change(exp, {assumptions.back(), expressions[temp_pos],
                                                    expressions[i]})->str.c_str() << std::endl;
                    }
                    break;
                }
                case 3: {
                    auto impl_expr = dynamic_cast<impl *>(expressions[i].get());
                    assert(impl_expr->right->type == EXPR_TYPE::ANY);
                    auto any_expr = dynamic_cast<any *>(impl_expr->right.get());
                    for (auto &rule : parser::load_proof("rules/A_B_@C.txt")) {
                        out << parser::change(rule, {assumptions.back(), impl_expr->left, any_expr->curr,
                                                     any_expr->var})->str.c_str() << std::endl;
                    }
                    break;
                }
                case 4: {
                    auto impl_expr = dynamic_cast<impl *>(expressions[i].get());
                    assert(impl_expr->left->type == EXPR_TYPE::EXIST);
                    auto exist_expr = dynamic_cast<exist *>(impl_expr->left.get());
                    for (auto &rule : parser::load_proof("rules/A_exB_C.txt")) {
                        out << parser::change(rule, {assumptions.back(), exist_expr->curr, impl_expr->right,
                                                     exist_expr->var})->str.c_str() << std::endl;
                    }
                    break;
                }
                default: {
                    out << expressions[i]->str.c_str() << std::endl;
                }
            }
        }
    }
    in.close();
    out.close();
    return 0;
}