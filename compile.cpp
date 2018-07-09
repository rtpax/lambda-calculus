#include "compile.h"

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
    component* temp_node = nullptr;

    std::vector<package*> pkgs;
    std::vector<component*> parens;
    std::vector<std::string> files;
    
    
    for(std::vector<token>::iterator tik = tok.begin(); tik != tok.end(); ++tik) {
        switch(tik->tt) {
        case token_type::lambda:
            std::cout << "L";
            temp_node = node;

            ++tik;
            if(tik->tt == token_type::dot || tik->tt == token_type::lparen) {
                emit_error("lambda cannot have zero arguments", tik->line_num, tik->filename);
            }

            while(tik->tt != token_type::dot && tik->tt != token_type::lparen) {
                if(tik->tt != token_type::identifier) {
                    emit_error("lambda argument must be an identifier", tik->line_num, tik->filename);
                }
                //TODO check that id name is valid
                node->lambda(id(tik->info), component());
                node = &node->lambda_out();
                std::cout << tik->info;
            }
            if(tik->tt == token_type::dot) {
                std::cout << ".";
            } else {
                temp_node->expr(component(std::move(temp_node)), component());//move version of expr must be used, else node will be invalidated.
                parens.push_back(temp_node);
                node->expr(component(), component());
                node = &node->expr_head();
                std::cout << "("
            }
            break;
        case token_type::dot:
            emit_error("stray '.' in program", tik->line_num, tik->filename);            
            break;
        case token_type::define:
            if(!definition.null()) {
                emit_error("cannot have multiple defintions in the same statement", tik->line_num, tik->filename);
            }
            if(node->is_id() && node->parent() == nullptr && node->scope() == &prepkg::global) {
                definition = node->id_name();
                *node = component();
            } else {
                if(!node.is_init()) {
                    emit_error("definition has no target", tik->line_num, tik->filename);
                } else {
                    emit_error("only identifiers can be defined", tik->line_num, tik->filename);
                }
            }
            std::cout << " := ";
            break;
        case token_type::inductive_definition:
            emit_error("inductive definitons not supported", , tik->line_num, tik->filename);
            std::cout << " ...= ";
            break;
        case token_type::identifier:
            if(node->bound_in_ancestor(tt->info)) {
                node->expr(component().id(tt->info), component());
                node = &node->expr_tail();
            } else {
                node->expr(component().id(tt->info, &prepkg::global), component());
                node = &node->expr_tail();
            }
            if(tik.info.size() == 0) {
                emit_error("zero length identifier is illegal", tik->line_num, tik->filename);
            } else if(tik.info.size() == 1) {
                std::cout << tik.info;
            } else {
                std::cout << "`" << tik.info << " ";
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
            if(pkgs.find(tik->info) == pkgs.end()) {
                global.add_package(tik->info);
                pkgs.push_back(tik->info);
            } else {
                emit_warning("began package twice", tik->line_num, tik->filename);
            }
            std::cout << tik->info << "::\n";
            break;
        case token_type::package_end:
            vector<package> index = pkgs.find(tik->info);
            if(index != pkgs.end()) {
                pkgs.remove(tik->info);
            } else {
                emit_warning("cannot end package since it was not begun", tik->line_num, tik->filename);
            }
            std::cout << "::" << tik->info << "\n";
            break;
        case token_type::package_scope:
            package * scope = global.get_package(tik->info);
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
                node->expr(component().id(tt->info, scope), component());
                node = &node->expr_tail();
                
            }
            break;
        case token_type::lparen:
            node->expr(component(), component());
            parens.push_back(node);
            node = node->expr_head();
            std::cout << "(";
            break;
        case token_type::rparen:
            if(parens.empty()) {
                emit_error("encountered ')' with no matching '('", tik->line_num, tik->filename)
            } else {
                node = parens.back();
                parens.pop_back();
                std::cout << ")";
            }
            break;
        case token_type::newline:
            if(parens.empty() && definition) {

            }
            std::cout << "\n";
            break;
        case token_type::none:
            std::cout << "[[token_type::none]]";
            break;
        }
    }

    return 1;


    return 1;
}

int evaluate_line(std::vector<token> line, int max_steps) {

}

}




/*

Lxy.xy(ba)

L-x
L-y
e-e-x
| y
e-b
a

L-x
L-y
*^

L-x
L-y
x^

L-x
L-y
e^-x
y

L-x
L-y
e-e-x
| y
*^





*/

