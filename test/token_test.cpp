#include "../token.h"
#include <fstream>

bool is_integer_string(std::string in) {
    if(in.size() == 0) {
        return false;
    }
    for(size_t i = 0; i < in.size(); ++i) {
        if(in[i] < '0' || in[i] > '9') {
            return false;
        }
    }
    return true;
}

int token_test() {
    std::ifstream ifs("./test/test.lc", std::ifstream::in);
    if(!ifs.is_open()) {
        std::cerr << "could not open file\n";
        return 0;
    }

    std::vector<token> tok = tokenize(ifs, "./test/test.lc");

    ifs.close();
    
    for(token tik : tok) {
        switch(tik.tt) {
        case token_type::lambda:
            std::cout << "L";
            break;
        case token_type::dot:
            std::cout << ".";
            break;
        case token_type::define:
            std::cout << " := ";
            break;
        case token_type::inductive_definition:
            std::cout << " ...= ";
            break;
        case token_type::identifier:
            if(is_integer_string(tik.info)) {
                std::cout << tik.info << " ";
            } else if(tik.info.size() == 1) {
                std::cout << tik.info;
            } else {
                std::cout << "`" << tik.info << " ";
            }
            break;
        case token_type::file:
            std::cout << "\"" << tik.info << "\"";
            break;
        case token_type::package_begin:
            std::cout << tik.info << "::\n";
            break;
        case token_type::package_end:
            std::cout << "::" << tik.info << "\n";
            break;
        case token_type::package_scope:
            std::cout << ":" << tik.info << ":";
            break;
        case token_type::lparen:
            std::cout << "(";
            break;
        case token_type::rparen:
            std::cout << ")";
            break;
        case token_type::newline:
            std::cout << "\n";
            break;
        case token_type::none:
            std::cout << "[[token_type::none]]";
            break;
        }
    }

    return 0;
}

int main() {
    token_test();
    return 0;
}