#ifndef LAMBDA_TOKEN_H
#define LAMBDA_TOKEN_H

#include <vector>
#include <iostream>

namespace lambda {

/**
 * indicates the type of token stored by a lambda::token object 
 **/
enum class token_type {
    lambda,
    dot,
    define,
    lazy_define,
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

/**
 * stores information about each token encountered in a file
 **/
struct token {
    token_type tt;
    std::string info;
    int line_num;
    std::string filename;
};

/**
 * tokenizes all data in a stream
 * 
 * note that any files referenced in this are not tokenized
 **/
std::vector<token> tokenize(std::istream& in, std::string filename);
/**
 * tokenizes a single line
 **/
std::vector<token> tokenize_line(std::string line, int line_num, std::string filename);

/**
 * returns true if `c` is a whitespace character
 **/
inline bool is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\r' || c=='\n');
}

/**
 * returns true if `c` is a numerical digit (0-9)
 **/
inline bool is_num(char c) {
    return (c >= '0' && c <= '9');
}

/**
 * returns true if `c` is a valid character that could occur in a package (a-zA-z0-9_-)
 **/
inline bool is_valid_pkg_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_') || (c == '-');
}

/**
 * returns true iff `c` is a character that could indicate the end of a name started with `\``
 **/
inline bool is_name_break(char c) {
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


#endif //LAMBDA_TOKEN_H