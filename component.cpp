#include "component.h"
#include "token.h"

namespace lambda {

int component::replace_ids(std::string replace_name, const component& replace, package * replace_scope) {
    if (is_id()) {
        if (id_name() == replace_name && scope() == replace_scope) {
            copy_preserve_parent(replace);
            return 1;
        } else {
            return 0;
        }
    } else if (is_expr()) {
        return expr_head().replace_ids(replace_name, replace, replace_scope) + 
            expr_tail().replace_ids(replace_name, replace, replace_scope);
    } else if (is_lambda()) {
        if(replace_scope == &prepkg::bound && lambda_arg().id_name() == replace_name) {
            return 0;
        } else {
            return lambda_out().replace_ids(replace_name, replace, replace_scope);
        }
    }
    throw std::logic_error("component not a lambda, expression, or identifier");
}

int component::evaluate_expression() {
    if(expr_head().is_id() && expr_head().scope() != &prepkg::bound) {
        const component * id_value = nullptr;
        if(expr_head().scope() == &prepkg::global) {
            id_value = global.get_value(expr_head().id_name());
        } else {
            id_value = expr_head().scope()->get_value(expr_head().id_name());
        }
        if(id_value != nullptr) {
            expr_head().copy_preserve_parent(*id_value);
        } else {
            return 0;
        }
    }
    if (expr_head().is_lambda()) {
        component * node = &expr_head().lambda_out();
        while(node->is_lambda()) {
            if(expr_tail().bound_in_argument(node->lambda_arg().id_name())) {
                unsigned rev = 0;
                std::string base = base_name(node->lambda_arg().id_name());
                std::string alt = alt_name(base, rev);
                while(expr_tail().bound_in_argument(alt) || node->lambda_out().bound_in_output(alt)) {
                    ++rev;
                    alt = alt_name(base, rev);
                }
                node->lambda_out().replace_ids(node->lambda_arg().id_name(), component().id(alt), &prepkg::bound);
                node->lambda_arg().id_name() = alt;
            }
            node = &node->lambda_out();
        }

        expr_head().lambda_out().replace_ids(expr_head().lambda_arg().id_name(), expr_tail());
        copy_preserve_parent(expr_head().lambda_out());
        return 1;
    }
    return 0;
}

int component::evaluate_step() {
    if (is_expr()) {
        if(expr_head().evaluate_step()) {
            return 1;
        } else {
            if(expr_head().is_lambda()) {
                return evaluate_expression();
            } else if (expr_head().is_id()){
                return 0;
            } else {
                return 0;
            }
        }
    } else if (is_lambda()) {
        return 0;
    } else if (scope() == &prepkg::global) {
        return 0;
    } else if (scope() != &prepkg::bound) {
        return 0;
    } else {
        return 0;
    }
}

int component::simplify_step() {
    if (is_expr()) {
        if(expr_head().simplify_step()) {
            return 1;
        } else {
            if(expr_head().is_lambda()) {
                return evaluate_expression();
            } else {
                return expr_tail().simplify_step();
            }
        }
    } else if (is_lambda()) {
        return lambda_out().simplify_step();
    } else if (scope() == &prepkg::global) {
        const component * value = global.get_value(id_name());
        if(value == nullptr) {
            return 0;
        } else {
            copy_preserve_parent(*value);
            return 1;
        }
    } else if (scope() != &prepkg::bound) {
        return 0;
    } else {
        const component * value = scope()->get_value(id_name());
        if(value == nullptr) {
            return 0;
        } else {
            copy_preserve_parent(*value);
            return 1;
        }
    }
}

std::string component::to_string() const {
    std::string ret = "";

    if (is_lambda()) {
        const component * node = this;
        ret += "L";
        ret += lambda_arg().to_string();
        node = &lambda_out();
        while(node->is_lambda()) {
            ret += node->lambda_arg().to_string();
            node = &node->lambda_out();
        }
        ret += ".";
        ret += node->to_string();
    } else if (is_expr()) {
        if(expr_head().is_lambda()) {
            ret += "(";
            ret += expr_head().to_string();
            ret += ")";
        } else {
            ret += expr_head().to_string();
        }
        if(expr_tail().is_lambda() || expr_tail().is_expr()) {
            ret += "(";
            ret += expr_tail().to_string();
            ret += ")";
        } else {
            ret += expr_tail().to_string();
        }
    } else if (is_id()) {
        switch(id_name().size()) {
        case 0:
            throw std::logic_error("identifier has name of length 0");
        case 1:
            if(id_name()[0] != 'L') {
                ret += id_name();
                break;
            }
            //else fallthrough
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
        if(ret[i] == ' ' && (i < ret.size() ? is_name_break(ret[i + 1]) : 1)) {
            ret.erase(i, 1);
        } else {
            ++i;
        }
    }

    return ret;
}



const component * package::get_value(std::string key) {
    try { //holding out for c++20 map::contains
        return &values.at(key);
    } catch(std::out_of_range) {
        return nullptr;
    }
}
void package::add_value(std::string key, const component& to_add) {
    values[key] = to_add;
}

const component * global_package::get_value(std::string key) {
    try { //holding out for c++20 map::contains
        return &values.at(key);
    } catch(std::out_of_range) {
        for(std::pair<const std::string,package>& p : packages) {
            const component * ret = p.second.get_value(key);
            if(ret != nullptr) {
                return ret;
            }
        }
    }
    return nullptr;
}
void global_package::add_value(std::string key, const component& to_add) {
    values[key] = to_add;
}

package * global_package::add_package(std::string key) {
    return &packages[key];
}

package * global_package::get_package(std::string key) {
    try {
        return &packages.at(key);
    } catch (std::out_of_range) {
        return nullptr;
    }
}

}


























