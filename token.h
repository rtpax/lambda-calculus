#ifndef LAMBDA_TOKEN_H
#define LAMBDA_TOKEN_H

#include <vector>
#include <iostream>

namespace lambda {

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
    newline,
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


bool is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\r' || c=='\n');
}

bool is_num(char c) {
    return (c >= '0' && c <= '9');
}

bool is_valid_pkg_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_') || (c == '-');
}

bool is_name_break(char c) {
    switch(c) {
    case '(':
    case ')':
    case ' ':
    case '\t':
    case '\r':
    case '\n':
    case '`':
    case ';':
    case '.':
    case ':':
        return true;
    default:
        return false;    
    }
}


}


#endif