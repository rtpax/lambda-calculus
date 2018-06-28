#include "token.h"
#include <stdexcept>
#include "emit.h"
#include <stdio.h>

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

token get_package(const std::string& check, int line_num, const char * filename) {
    size_t i = 0;
    token ret{token_type::none, "", line_num, filename};

    for(; i < check.size() && is_whitespace(check[i]); ++i);

    if(i == check.size())
        return ret;

    if(check[i] == ':' && i + 1 < check.size() && check[i + 1] == ':') {
        i += 2;

        for(bool has_warned = false; i < check.size(); ++i) {
            if(is_whitespace(check[i])) {
                ++i;
                break;
            } else if(is_valid_pkg_char(check[i])) {
                ret.info += check[i];
            } else if(!has_warned){
                has_warned = true;
                emit_error("invalid characters in package name", line_num, filename);
            }
        }
        
        for(bool has_warned = false; i < check.size(); ++i) {
            if(!is_whitespace(check[i]) && !has_warned) {
                has_warned = true;
                emit_warning("ignoring extra characters after package name", line_num, filename);
            }
        }

        ret.tt = token_type::package_end;
    
    } else {
        bool proper_end = false;
        bool invalid_char = false;
        for(; i < check.size(); ++i) {
            if(check[i] == ';') {
                return token{token_type::none};
            } else if(check[i] == ':' && i + 1 < check.size() && check[i + 1] == ':') {
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
            emit_error("invalid characters in package name", line_num, filename);
        }
        if(proper_end) {
            ret.tt = token_type::package_begin;
        } else {
            return token{token_type::none};
        }

        for(; i < check.size(); ++i) {
            if(check[i] == ';')
                break;
            if(!is_whitespace(check[i])) {
                emit_warning("ignoring extra characters after package name", line_num, filename);
                break;
            }
        }
    }

    return ret;
}

token get_include(std::string check, int line_num, const char * filename) {
    token ret{token_type::file,"",line_num,filename};
    size_t i = 0;
    for(; i < check.size() && is_whitespace(check[i]); ++i);

    if(!(i < check.size() && check[i] == '"')) {
        return token{token_type::none};
    }
    ++i;

    for(; i < check.size() && check[i] != '"'; ++i) {
        ret.info += check[i];
    }

    if(!(i < check.size() && check[i] == '"')) {
        emit_error("no closing '\"' in file name", line_num, filename);
        return token{token_type::none};
    }
    ++i;

    for(; i < check.size(); ++i) {
        if(check[i] == ';')
            break;
        if(!is_whitespace(check[i])) {
            emit_warning("ignoring extra characters after file name", line_num, filename);
            break;
        }
    }

    return ret;
}
//┌┘
//╪╤╧╘
//╫╓╒╙╥╨╬═╠╦╩╔╚╟╞┼─├┬┴└┐╛╜╝╗║╣╕╖╢╡┤│

std::vector<token> tokenize_line(std::string line, int line_num, const char * filename, int& comment) {
    std::vector<token> out;

    size_t i = 0;
    for(; i < line.size() && comment > 0; ++i) {
        if(line.size() > i + 2 &&
                line[i + 0] == ':' &&
                line[i + 1] == ':' &&
                line[i + 2] == ';') {
            i += 2;
            --comment;
        }
    }

    line = line.substr(i);
    i = 0;

    token pkg = get_package(line, line_num, filename);
    if(pkg.tt != token_type::none) {
        out.push_back(pkg);
        return out;
    }

    token inc = get_include(line, line_num, filename);
    if(inc.tt != token_type::none) {
        out.push_back(inc);
        return out;
    }


    for(; i < line.size(); ++i) {
        token id;
        switch(line[i]) {
        case 'L':
            out.push_back(token{token_type::lambda});
            break;
        case '.':
            if(line.size() > i + 3 &&
                    line[i + 1] == '.' &&
                    line[i + 2] == '.' &&
                    line[i + 3] == '=') {
                out.push_back(token{token_type::inductive_definition, "", line_num, filename});
                i += 3;
            } else {
                out.push_back(token{token_type::dot, "", line_num, filename});
            }
            break;
        case ':':
            if(line[i + 1] == '=') {
                out.push_back(token{token_type::define, "", line_num, filename});
                ++i;
            } else {
                //package scope
            }
            break;
        case '#':
            out.push_back(token{token_type::identifier, "#", line_num, filename});
            break;
        case '`':
            id = {token_type::identifier, "", line_num, filename};
            for(i = i + 1; i < line.size() && !is_name_break(line[i]); ++i) {
                id.info += line[i];
            }
            --i;
            if(id.info.size() == 0) {
                emit_warning("ignoring zero length name", line_num, filename);
            } else {
                out.push_back(id);
            }
            break;
        case '(':
            out.push_back(token{token_type::lparen, "", line_num, filename});
            break;
        case ')':
            out.push_back(token{token_type::rparen, "", line_num, filename});
            break;
        case ';':
            if(line.size() > i + 2 &&
                    line[i + 1] == ':' &&
                    line[i + 2] == ':') {
                i += 2;
                ++comment;
                for(; i < line.size() && comment > 0; ++i) {
                    if(line.size() > i + 2 &&
                            line[i + 0] == ':' &&
                            line[i + 1] == ':' &&
                            line[i + 2] == ';') {
                        i += 2;
                        --comment;
                    }
                }
            } else {
                i = line.size();
            }
            break;
        default:
            if(is_num(line[i])) {
                id = {token_type::identifier, "", line_num, filename};
                for(; i < line.size() && is_num(line[i]); ++i) {
                    id.info += line[i];
                }
                --i;
                out.push_back(id);
            }
            else if(!is_whitespace(line[i])) {
                out.push_back(token{token_type::identifier, std::string("") + line[i], line_num, filename});
            }
            break;
        }
    }
    out.push_back(token{token_type::newline, "", line_num, filename});
    return out;
}

std::vector<token> tokenize(std::istream& in, const char * filename) {
    std::vector<token> out;

    int comment = 0;
    int line_num = 0;

    std::string line;
    std::getline(in, line);
    ++line_num;

    while(!in.eof()) {
        std::vector<token> line_tok = tokenize_line(line, line_num, filename, comment);
        out.insert(out.end(), line_tok.begin(), line_tok.end());

        std::getline(in, line);
        ++line_num;
    }

    return out;
}



