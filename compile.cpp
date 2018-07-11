#include "compile.h"
#include "emit.h"
#include <fstream>
#include <algorithm>


namespace lambda {


int load_file(std::string filename) {
    static const int TIMEOUT = 100000;
    
    std::ifstream ifs(filename, std::ifstream::in);
    if(!ifs.is_open()) {
        return 0;
    }

    std::vector<token> tok = tokenize(ifs, filename);
    if(!tok.empty())
        tok.push_back(token{token_type::newline,"",tok.back().tt == token_type::newline ? tok.back().line_num + 1 : tok.back().line_num,filename});
 
    ifs.close();

    nullable<std::string> definition;
    bool lazy_def = true;

    component* node = new component();

    std::vector<std::string> pkgs;
    std::vector<component*> parens;
    std::vector<std::string> files;
    
    for(std::vector<token>::iterator tik = tok.begin(); tik != tok.end(); ++tik) {
        std::vector<std::string>::iterator index;
        package * scope;
        component* oldnode = nullptr;
        bool bound, empty;
        switch(tik->tt) {
        case token_type::lambda:
            std::cout << "L";
            if(node->is_init() && !node->is_lambda()) {
                parens.push_back(node);
                node = new component();
            }
            oldnode = node;
            while (node->is_lambda()) {
                node = &node->lambda_out();
            }

            ++tik;
            if(tik->tt == token_type::dot || tik->tt == token_type::lparen) {
                emit_error("lambda cannot have zero arguments", tik->line_num, tik->filename);
            }

            while(tik->tt != token_type::dot && tik->tt != token_type::lparen && tik != tok.end()) {
                if(tik->tt != token_type::identifier) {
                    emit_error("lambda argument must be an identifier", tik->line_num, tik->filename);
                }
                //TODO check that id name is valid
                node->lambda(component().id(tik->info), component());
                node = &node->lambda_out();
                if(tik->info.size() == 0) {
                    emit_error("zero length identifier", tik->line_num, tik->filename);
                    break;
                } else if((tik->info.size() == 1 || tik->info[1] == '\'') && tik->info[0] != 'L') {
                    std::cout << tik->info;
                } else {
                    std::cout << "`" << tik->info << " ";
                }
                ++tik;
            }
            node = oldnode;
            if(tik == tok.end()) {
                emit_error("program ended before statement completion", tok.back().line_num, tok.back().filename);
            } if(tik->tt == token_type::dot) {
                std::cout << ".";
            } else /*lparen*/ {
                parens.push_back(new component());
                std::cout << "(";
            }
            break;
        case token_type::dot:
            emit_error("stray '.' in program", tik->line_num, tik->filename);
            break;
        case token_type::define:
        case token_type::lazy_define:
            if(!parens.empty()) {
                emit_error("cannot have definition inside parentheses", tik->line_num, tik->filename);
                break;
            }
            if(!definition.null()) {
                emit_error("cannot have multiple defintions in the same statement", tik->line_num, tik->filename);
                break;
            }
            if(node->is_id() && node->parent() == nullptr && node->scope() == &prepkg::global) {
                definition = node->id_name();
                if(tik->tt == token_type::define) {
                    lazy_def = false;
                } else {
                    lazy_def = true;
                }    
                node->clear();
            } else {
                if(!node->is_init()) {
                    emit_error("definition has no target", tik->line_num, tik->filename);
                } else {
                    emit_error("only identifiers can be defined", tik->line_num, tik->filename);
                }
            }
            if(tik->tt == token_type::define) {
                std::cout << " := ";
            } else {
                std::cout << " => ";
            }
            break;
        case token_type::inductive_definition:
            emit_error("inductive definitons not supported", tik->line_num, tik->filename);
            std::cout << " ...= ";
            break;
        case token_type::identifier:
            if(tik->info.size() == 0) {
                emit_error("zero length identifier", tik->line_num, tik->filename);
                break;
            } else if((tik->info.size() == 1 || tik->info[1] == '\'') && tik->info[0] != 'L') {
                std::cout << tik->info;
            } else {
                std::cout << "`" << tik->info << " ";
            }
            bound = node->is_lambda() ? node->lambda_has_arg(tik->info) : 0;
            for(component * par : parens) { 
                if(bound)
                    break;
                bound = par->is_lambda() ? par->lambda_has_arg(tik->info) : 0;
            }
            if(bound) {
                node->append(component().id(tik->info));
            } else {
                node->append(component().id(tik->info, &prepkg::global));
            }
            break;
        case token_type::file:
            if(/*file not opened yet*/1) {
                load_file(tik->info);//TODO search for the file in viable spots before including
            } else {
                emit_warning("skipping repeated file", tik->line_num, tik->filename);
            }
            std::cout << "\"" << tik->info << "\"";
            break;
        case token_type::package_begin:
            //TODO check if package has a valid name
            if(std::find(pkgs.begin(), pkgs.end(), tik->info) == pkgs.end()) {
                global.add_package(tik->info);
                pkgs.push_back(tik->info);
            } else {
                emit_warning("began package twice", tik->line_num, tik->filename);
            }
            std::cout << tik->info << "::\n";
            break;
        case token_type::package_end:
            index = std::find(pkgs.begin(), pkgs.end(), tik->info);
            if(index != pkgs.end()) {
                pkgs.erase(index);
            } else {
                emit_warning("cannot end package since it was not begun", tik->line_num, tik->filename);
            }
            std::cout << "::" << tik->info << "\n";
            break;
        case token_type::package_scope:
            scope = global.get_package(tik->info);
            if(scope == nullptr) {
                emit_warning("could not find the specified package. defaulting to global scope.", tik->line_num, tik->filename);
                scope = &prepkg::global;
            }
            std::cout << ":" << tik->info << ":";
            ++tik;
            if(tik->tt != token_type::identifier) {
                emit_error("package scope must be followed by an identifier", tik->line_num, tik->filename);
                --tik;
            } else {
                const component * value = scope ? scope->get_value(tik->info) : global.get_value(tik->info);
                if(value == nullptr) {
                    emit_warning("could not find variable in the specified scope", tik->line_num, tik->filename);
                }
                if(tik->info.size() == 0) {
                    emit_error("zero length identifier", tik->line_num, tik->filename);
                    break;
                } else if((tik->info.size() == 1 || tik->info[1] == '\'') && tik->info[0] != 'L') {
                    std::cout << tik->info;
                } else {
                    std::cout << "`" << tik->info << " ";
                }
                node->append(component().id(tik->info, scope ?: &prepkg::global));
            }
            break;
        case token_type::lparen:
            parens.push_back(node);
            node = new component();
            std::cout << "(";
            break;
        case token_type::rparen:
            if(parens.empty()) {
                emit_error("encountered ')' with no matching '('", tik->line_num, tik->filename);
            } else {
                oldnode = node;
                node = parens.back();
                parens.pop_back();
                if(oldnode->is_lambda() && !node->is_init()) {
                    node->expr(std::move(*oldnode), component());
                } else {
                    node->append(std::move(*oldnode));
                }
                delete oldnode;

                std::cout << ")";
            }
            break;
        case token_type::newline:
            empty = 1;
            for(component * check : parens) {
                empty = check->is_lambda();
                if(!empty)
                    break;
            }
            if(empty) {
                while(!parens.empty()) {
                    oldnode = node;
                    node = parens.back();
                    parens.pop_back();
                    node->append(std::move(*oldnode));
                    delete oldnode;
                }
                if(!node->is_init()) {
                    break;
                }
                if(node->is_expr() && !node->expr_tail().is_init()) {
                    node->copy_preserve_parent(std::move(node->expr_head()));
                }
                if(!node->is_deep_init()) {
                    emit_error("incomplete statement", tik->line_num, tik->filename);
                    break;
                }
                int timeout = node->evaluate(TIMEOUT);
                std::cout << "\n    " << node->to_string() << "[" << (TIMEOUT - timeout) << "]";
                if(timeout == 0) {
                    emit_warning("stopped evaluating lambda after timeout",tik->line_num,tik->filename);
                }

                if(definition.null() || !lazy_def) {
                    if(timeout != 0) {
                        timeout = node->simplify(TIMEOUT);
                        std::cout << "\n    " << node->to_string() << "[" << (TIMEOUT - timeout) << "]";
                        if(timeout == 0) {
                            emit_warning("stopped evaluating lambda after timeout",tik->line_num,tik->filename);
                        }
                    }
                }
                if(!definition.null()) {
                    for (std::string p : pkgs) {
                        if (global.get_package(p)->get_value(definition) != nullptr) {
                            emit_warning("redefining variable within package", tik->line_num, tik->filename);
                        }
                        global.get_package(p)->add_value(definition, *node);
                    }
                    if (pkgs.empty()) {
                        if (global.get_value(definition) != nullptr) {
                            emit_warning("redefining variable in global space", tik->line_num, tik->filename);
                        }
                        global.add_value(definition, *node);
                    }
                    definition.nullify();
                }
                node->clear();
            } //else ignore
            std::cout << "\n";
            break;
        case token_type::none:
            emit_error("unrecognized token", tik->line_num, tik->filename);
            break;
        }
    }
    if(!tok.empty() && !parens.empty())
        emit_warning("program ended with incomplete statement", tok.back().line_num, filename);


    return 1;
}

int evaluate_line(std::vector<token> line, int max_steps) {
    return 0;
}

}


using namespace lambda;

int main(int argc, char ** argv) {
    if(argc <= 1) {
        std::cout << "must input a file";
        return 1;
    }
    load_file(argv[1]);
    return 0;
}



