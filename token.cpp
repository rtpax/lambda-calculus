#include "token.h"
#include <stdexcept>

bool is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\r' || c=='\n')
}

bool is_valid_pkg_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_') || (c == '-');
}

token get_package(const std::string& check, int line_num) {
    size_t i = 0
    token ret{token_type::none, ""};

    for(; i < line.size() && is_whitespace(check[i]); ++i);

    if(i = line.size())
        return ret;

    if(check[i] == ':' && i + 1 < check.size() && check[i + 1] == ':') {
        i+=2
        
        for(bool has_warned = false; i < check.size(); ++i) {
            if(is_whitespace(check[i])) {
                ++i;
                break;
            } else if(is_valid_pkg_char(check[i])) {
                ret.info += check[i];
            } else if(!has_warned){
                has_warned = true;
                emit_error("invalid characters in package name", line_num);
            }
        }
        
        for(bool has_warned = false; i < check.size(); ++i) {
            if(!is_whitespace(check[i]) && !has_warned) {
                has_warned = true;
                emit_warning("ignoring extra characters after package name", line_num);
            }
        }

        ret.tt = token_type::package_end;
    
    } else {
        bool proper_end = false;
        bool invalid_char = false;
        for(; i < line.size(); ++i) {
            if(check[i] == ':' && i + 1 < check.size() && check[i + 1] == ':') {
                proper_end = true;
                i += 2;
                break;
            } else if (is_valid_pkg_char(check[i])) {
                ret.info += check[i];
            } else {
                invalid_char = true;
            }
        }

        if(proper_end && invalid_char) {
            emit_error("invalid characters in package name", line_num);
        }
        if(proper_end) {
            ret.tt = token_type::package_begin;
        } else {
            return token{token_type::none};
        }

        for(bool has_warned = false; i < check.size(); ++i) {
            if(!is_whitespace(check[i]) && !has_warned) {
                has_warned = true;
                emit_warning("ignoring extra characters after package name", line_num);
            }
        }
    }

    return ret;
}

std::vector<token> tokenize(std::istream in) {
    std::vector<token> out;

    int paren = 0;
    int comment = 0;
    int line_num = 1;

    while(!in.eof()) {
        std::string line;
        std::getline(in, line);

        token pkg = get_package(line, line_num);
        if(pkg.tt != token_type::none) {
            out.push_back(pkg);
            continue;
        }

        for(size_t i = 0; i < line.size(); ++i)
            switch(line[i]) {
            case 'L':
                out.push_back(token{token_type::lambda})
                break;
            case '.'
                //dot or inductive definition
                break;
            case ':'
                //define or package scope
                break;
            case '≡': 
                out.push_back(token{token_type::define})
            case '#':
                //[next] inductive identifier
            case '`':
                //identifier
            case '"':
                //file
            case '(':
                out.push_back(token{token_type::lparen});
                ++paren;
                break;
            case ')':
                if(paren == 0) {
                    emit_error("unbalance parentheses", line_num);
                }
                else {
                    --paren;
                    out.push_back(token{token_type::rparen});
                }
                break;
            case '\n'
                ++line_num;
                if(paren == 0) {

                } else {

                }
                break;
            default:
                //identifier whitespace ...
                break;
            }
        }
    }
}