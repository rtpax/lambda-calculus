#include "./compile.h"

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

void print_expression(const_ref_component e) {
    for(const component sub : *e.value.e) {
        switch(sub->type) {
        case component_type::EXPRESSION:
            std::cout << "(";
            print_expression(sub);
            std::cout << ")";
            break;
        case component_type::IDENTIFIER:
            print_identifier(c);
            break;
        case component_type::LAMBDA:
            std::cout << "(";
            print_lambda(c);
            std::cout << ")";
            break;
        default:
            throw std::logic_error("could not print component of indeterminate type");
        }    
    }
}

void print_identifier(const_ref_component i) {
    if(is_integer_string(i.value.i->info)) {
        std::cout << i.value.i->info << " ";
    } else if(i.value.i->info.size() == 1) {
        std::cout << i.value.i->info;
    } else {
        std::cout << "`" << i.value.i->info << " ";
    }
}

void print_lambda(const_ref_component l) {
    std::cout << "L";
    for(identifier& i : l.value.l->arguments) {
        print_identifier(i);
    }
    std::cout << ".";
    print_expression(l.value.l->output);
}

void print_component(const_ref_component c) {
    switch(c.type) {
    case component_type::EXPRESSION:
        print_expression(c);
        break;
    case component_type::IDENTIFIER:
        print_identifier(c);
        break;
    case component_type::LAMBDA:
        print_lambda(c);
        break;
    default:
        throw std::logic_error("could not print component of indeterminate type");
    }    
}

int main() {
    lambda base;
    identifier_base_arg_1{ base,bound, }
}