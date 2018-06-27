#include <vector>
#include <iostream>

enum class token_type {
    lambda,
    dot,
    define,
    inductive_definition,
    inductive_identifier,
    next_inductive_identifier,
    identifier,
    file,
    package_begin,
    package_end,
    lparen,
    rparen,
    newline, //newlines are ignored if parentheses are unbalanced
    none
};

enum comparator {
    lt,gt,lte,gte,eq,nop;
}

struct token_info {};

struct token {
    token_type tt;
    std::string info;
};

template<class character>
std::vector<token> tokenize(std::basic_istream<character> in);





