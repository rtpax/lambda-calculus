#include "component.h"

namespace lambda {

int component::replace_ids(std::string replace_name, const component& replace, package * replace_space) {
    if (is_id()) {
        if (id_name() == replace_name && space() == replace_space) {
            copy_preserve_parent(replace);
            return 1;
        } else {
            return 0;
        }
    } else if (is_expr()) {
        return expr_head().replace_ids(replace_name, replace, replace_space) + 
            expr_tail().replace_ids(replace_name, replace, replace_space);
    } else if (is_lambda()) {
        if(replace_space == &space::bound && lambda_arg().id_name() == replace_name) {
            return 0;
        } else {
            return lambda_out().replace_ids(replace_name, replace, replace_space);
        }
    }
    throw std::logic_error("component not a lambda, expression, or identifier");
}

int component::evaluate_expression() {
    if(expr_head().is_id() && expr_head().space() != &space::bound) {
        const component * id_value = expr_head().space()->get_value(expr_head().id_name());
        if(id_value != nullptr) {
            expr_head().copy_preserve_parent(*id_value);
        } else {
            return 0;
        }
    }
    if (expr_head().is_lambda()) {
        expr_head().lambda_out().replace_ids(expr_head().lambda_arg().id_name(), expr_tail());
        copy_preserve_parent(expr_head().lambda_out());
    }
}

int component::evaluate_step() {
    if (is_expr()) {
        if(expr_head().evaluate_step()) {
            return 1;
        } else {
            if(expr_head().is_lambda()) {
                if(expr_tail().evaluate_step()) {
                    return 1;
                } else {
                    return evaluate_expression();
                }
            } else if (expr_head().is_lambda()){
                return 0;
            }
        }
    } else if (is_lambda()) {
        return 0;
    } else if (space() == &space::global) {
        return 0;
    } else if (space() != &space::bound) {
        return 0;
    } else {
        return 0;
    }
}

std::string component::to_string() {
    component& node = *this;
    std::string ret = "";

    if (is_lambda()) {
        ret += "L";
        ret += lambda_arg().id_name();
        node = lambda_out();
        while(node.is_lambda()) {
            ret += node.lambda_arg().id_name();
            node = node.lambda_out();
        }
        ret += node.to_string();
    } else if (is_expr()) {
        if(expr_head().is_lambda()) {
            ret += "(";
            ret += expr_head().to_string();
            ret += ")";
            ret += expr_tail().to_string();
        } else if (expr_head().is_expr()) {
            ret += expr_head().to_string();
            ret += expr_tail().to_string();
        }
    } else if (is_id()) {
        switch(id_name().size()) {
        case 0:
            throw std::logic_error("identifier has name of length 0");
        case 1:
            ret += id_name();
            break;
        default:
            ret += "`";
            ret += id_name();
            ret += " ";
            break;
        }
    } else {
        throw std::logic_error("component not a lambda, expression, or identifier");
    }

    for(size_t i = 0; i < ret.size();) {
        //remove unnecessary whitespace
        ++i;
    }

    return ret;
}

}