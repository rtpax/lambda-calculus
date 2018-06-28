#include <vector>
#include <iostream>

enum class token_type {
    lambda,
    dot,
    define,
    inductive_definition,
    identifier,
    file,
    package_begin,
    package_end,
    package_scope,
    lparen,
    rparen,
    newline, //newlines are ignored if parentheses are unbalanced
    none
};


struct token_info {};

struct token {
    token_type tt;
    std::string info;
    int line_num;
    const char * filename;
};

std::vector<token> tokenize(std::istream& in, const char * filename);
std::vector<token> tokenize_line(std::string line, int line_num, char * filename);






