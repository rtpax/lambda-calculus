#include "compile.h"
#include "emit.h"
#include <fstream>
#include <algorithm>

namespace lambda {

int load_file(std::string filename) {
    std::ifstream ifs(filename, std::ifstream::in);
    if(!ifs.is_open()) {
        return 0;
    }

    std::vector<token> tok = tokenize(ifs, "filename");

    ifs.close();

    nullable<std::string> definition;

    component* node = new component();

    std::vector<std::string> pkgs;
    std::vector<component*> parens;
    std::vector<std::string> files;
    
    for(std::vector<token>::iterator tik = tok.begin(); tik != tok.end(); ++tik) {
        std::vector<std::string>::iterator index;
        package * scope;
        component* oldnode = nullptr;
        bool bound;
        switch(tik->tt) {
        case token_type::lambda:
            std::cout << "L";
            parens.push_back(node);

            node->append(component());

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
            if(tik == tok.end()) {
                emit_error("program ended before statement completion", tok.back().line_num, tok.back().filename);
            } if(tik->tt == token_type::dot) {
                parens.pop_back();
                std::cout << ".";
            } else /*lparen*/ {
                std::cout << "(";
            }
            break;
        case token_type::dot:
            emit_error("stray '.' in program", tik->line_num, tik->filename);
            break;
        case token_type::define:
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
                *node = component();
            } else {
                if(!node->is_init()) {
                    emit_error("definition has no target", tik->line_num, tik->filename);
                } else {
                    emit_error("only identifiers can be defined", tik->line_num, tik->filename);
                }
            }
            std::cout << " := ";
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
            bound = node->bound_in_ancestor(tik->info);
            for(component * par : parens) { 
                if(bound)
                    break;
                bound = par->bound_in_ancestor(tik->info);
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
            if(tik->tt == token_type::identifier) {
                emit_error("package scope must be followed by an identifier", tik->line_num, tik->filename);
                --tik;
            } else {
                node->expr(component().id(tik->info, scope), component());
                node = &node->expr_tail();    
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

                node->append(std::move(*oldnode));
                delete oldnode;

                std::cout << ")";
            }
            break;
        case token_type::newline:
            if(parens.empty()) {
                if(!node->is_init()) {
                    break;
                }
                node = node->base_parent();
                if(!node->is_deep_init()) {
                    emit_error("incomplete statement", tik->line_num, tik->filename);
                    break;
                }
                if(definition.null()) {
                    int max_iter = 100;
                    bool stepped = 1;
                    while(stepped && max_iter > 0) {
                        std::cout << "\n    " << node->to_string();
                        stepped = node->evaluate_step();
                        if(!stepped)
                            stepped = node->simplify_step();
                        --max_iter;
                    }
                    node->clear();
                } else {
                    for(std::string p : pkgs) {
                        global.get_package(p)->add_value(definition, *node);
                        node->clear();
                    }
                    definition.nullify();
                }
            } //else ignore
            std::cout << "\n";
            break;
        case token_type::none:
            emit_error("unrecognized token", tik->line_num, tik->filename);
            std::cout << "[[token_type::none]]";
            break;
        }
    }


    return 1;
}

int evaluate_line(std::vector<token> line, int max_steps) {
    return 0;
}

}


using namespace lambda;

int main() {
    load_file("test/test.lc");
}