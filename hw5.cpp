#include <iostream>
#include <fstream>
#include <vector>

std::vector<std::string>
        begin = {"0=0->0=0->0=0",
                 "a=b->a=c->b=c",
                 "(a=b->a=c->b=c)->(0=0->0=0->0=0)->(a=b->a=c->b=c)",
                 "(0=0->0=0->0=0)->(a=b->a=c->b=c)",
                 "(0=0->0=0->0=0)->@c(a=b->a=c->b=c)",
                 "(0=0->0=0->0=0)->@b@c(a=b->a=c->b=c)",
                 "(0=0->0=0->0=0)->@a@b@c(a=b->a=c->b=c)",
                 "@c(a=b->a=c->b=c)",
                 "@c(a=b->a=c->b=c)->(a=b->a=a->b=a)",
                 "a=b->a=a->b=a",
                 "@a@b@c(a=b->a=c->b=c)",
                 "@a@b@c(a=b->a=c->b=c)->@b@c(a+0=b->a+0=c->b=c)",
                 "(0=0->0=0->0=0)->@a@b@c(a=b->a=c->b=c)",
                 "@b@c(a+0=b->a+0=c->b=c)->@c(a+0=a->a+0=c->a=c)",
                 "@b@c(a+0=b->a+0=c->b=c)",
                 "@c(a+0=a->a+0=c->a=c)",
                 "@c(a+0=a->a+0=c->a=c)->(a+0=a->a+0=a->a=a)",
                 "a+0=a->a+0=a->a=a",
                 "a+0=a",
                 "a+0=a->a=a",
                 "a=a",
                 "a=a->a=b->a=a",
                 "a=b->a=a",
                 "(a=b->a=a)->(a=b->a=a->b=a)->(a=b->b=a)",
                 "(a=b->a=a->b=a)->(a=b->b=a)",
                 "a=b->b=a",
                 "(a=b->b=a)->(0=0->0=0->0=0)->(a=b->b=a)",
                 "(0=0->0=0->0=0)->(a=b->b=a)",
                 "(0=0->0=0->0=0)->@b(a=b->b=a)",
                 "(0=0->0=0->0=0)->@a@b(a=b->b=a)",
                 "@a@b(a=b->b=a)",
                 "@a@b(a=b->b=a)->@b(x=b->b=x)",
                 "@b(x=b->b=x)",
                 "@b(x=b->b=x)->(x=y->y=x)",
                 "x=y->y=x",
                 "(x=y->y=x)->(0=0->0=0->0=0)->(x=y->y=x)",
                 "(0=0->0=0->0=0)->(x=y->y=x)",
                 "(0=0->0=0->0=0)->@y(x=y->y=x)",
                 "(0=0->0=0->0=0)->@x@y(x=y->y=x)",
                 "@x@y(x=y->y=x)"},
        step = {"a+b'=(a+b)'",
                "a+b'=(a+b)'->(A->B->A)->a+b'=(a+b)'",
                "(A->B->A)->a+b'=(a+b)'",
                "(A->B->A)->@b(a+b'=(a+b)')",
                "A->B->A",
                "@b(a+b'=(a+b)')",
                "@b(a+b'=(a+b)')->(a+o'=(a+o)')",
                "a+o'=(a+o)'",
                "@x@y(x=y->y=x)->@y((a+o')=y->y=(a+o'))",
                "@y((a+o')=y->y=(a+o'))",
                "@y((a+o')=y->y=(a+o'))->(a+o')=(a+o)'->(a+o)'=(a+o')",
                "(a+o')=(a+o)'->(a+o)'=(a+o')",
                "(a+o)'=(a+o')",
                "a=b->a=c->b=c",
                "(a=b->a=c->b=c)->(A->B->A)->(a=b->a=c->b=c)",
                "(A->B->A)->(a=b->a=c->b=c)",
                "(A->B->A)->@c(a=b->a=c->b=c)",
                "(A->B->A)->@b@c(a=b->a=c->b=c)",
                "(A->B->A)->@a@b@c(a=b->a=c->b=c)",
                "(A->B->A)",
                "@a@b@c(a=b->a=c->b=c)",
                "@a@b@c(a=b->a=c->b=c)->@b@c((a+o)'=b->(a+o)'=c->b=c)",
                "@b@c((a+o)'=b->(a+o)'=c->b=c)",
                "@b@c((a+o)'=b->(a+o)'=c->b=c)->@c((a+o)'=(a+o')->(a+o)'=c->(a+o')=c)",
                "@c((a+o)'=(a+o')->(a+o)'=c->(a+o')=c)",
                "@c((a+o)'=(a+o')->(a+o)'=c->(a+o')=c)->((a+o)'=(a+o')->(a+o)'=d'->(a+o')=d')",
                "((a+o)'=(a+o')->(a+o)'=d'->(a+o')=d')",
                "(a+o)'=d'->(a+o')=d'",
                "a+o=d",
                "a=b->a'=b'",
                "(a=b->a'=b')->(A->B->A)->(a=b->a'=b')",
                "(A->B->A)->(a=b->a'=b')",
                "(A->B->A)->@b(a=b->a'=b')",
                "(A->B->A)->@a@b(a=b->a'=b')",
                "@a@b((a=b)->(a'=b'))",
                "@a@b((a=b)->(a'=b'))->@b((a+o=b)->(a+o)'=b')",
                "@b((a+o=b)->(a+o)'=b')",
                "@b((a+o=b)->(a+o)'=b')->((a+o=d)->(a+o)'=d')",
                "(a+o=d)->(a+o)'=d'",
                "(a+o)'=d'",
                "a+o'=d'"},
        end = {"a+o=d",
               "(a+o=d)->(A->B->A)->(a+o=d)",
               "(A->B->A)->(a+o=d)",
               "(A->B->A)->@a(a+o=d)",
               "(A->B->A)",
               "@a(a+o=d)",
               "@a(a+o=d)->(q+o=f)",
               "q+o=f"};

int main() {
    std::cout << R"(Usage: "input_file" "output_file")" << std::endl;
    std::string input_file, output_file = "out.out";
    std::cin >> input_file >> output_file;
    std::ifstream in(input_file);
    std::ofstream out(output_file);
    size_t a_num, b_num, sum_num;
    std::string s;
    in >> a_num >> b_num;
    in.close();
    if (std::max(a_num, b_num) > 100) {
        out << std::max(a_num, b_num) << " greater than 100\n";
        out.close();
        return 0;
    }

    sum_num = a_num + b_num;
    auto a = "0" + std::string(a_num, '\'');
    auto b = "0" + std::string(b_num, '\'');
    auto sum = "0" + std::string(sum_num, '\'');

    out << "|-" << a << "+" << b << "=" << sum << std::endl;
    for (auto &str : begin) {
        out << str << std::endl;
    }
    std::string zero("0");
    std::string left("a");
    for (size_t i = 0; i < b_num; i++) {
        for (auto &str : step) {
            for (auto &c : str) {
                if (c == 'o') {
                    out << zero;
                } else if (c == 'd') {
                    out << left;
                } else {
                    out << c;
                }
            }
            out << std::endl;
        }
        zero += '\'';
        left += '\'';
    }

    for (auto &str : end) {
        for (auto &c : str) {
            if (c == 'o') {
                out << zero;
            } else if (c == 'd') {
                out << left;
            } else if (c == 'q') {
                out << a;
            } else if (c == 'f') {
                out << sum;
            } else {
                out << c;
            }
        }
        out << std::endl;
    }
    out.close();
    return 0;
}