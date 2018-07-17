#include "component.h"
#include "token.h"
#include <algorithm>

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
    int ret = 0;
    if(expr_head().is_id() && expr_head().scope() != &prepkg::bound) {
        const component * id_value = nullptr;
        if(expr_head().scope() == &prepkg::global) {
            id_value = global.get_value(expr_head().id_name());
        } else {
            id_value = expr_head().scope()->get_value(expr_head().id_name());
        }
        if(id_value != nullptr) {
            expr_head().copy_preserve_parent(*id_value);
            ret = 1;
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
    return ret;
}

int component::simplify_step() {
    if(evaluate_step()) {
        return 1;
    }

    if (is_expr()) {
        if(expr_head().is_lambda() || expr_head().is_id()) {
            if(evaluate_expression())
                return 1;
        }
        if(expr_head().simplify_step()) {
            return 1;
        } else {
            return expr_tail().simplify_step();
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
    } else if (scope() == &prepkg::bound) {
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

int component::evaluate(int timeout) {
    while(timeout > 0) {
        std::cout << "\n     " << to_string();
        --timeout;
        if(!evaluate_step())
            break;
    }
    return timeout;
}

int component::simplify(int timeout) {
    while(timeout > 0) {
        // std::cout << "\n     " << to_string();
        --timeout;
        if(!simplify_step())
            break;
    }
    return timeout;
}

bool component::has_unknown() const {
    std::vector<std::string> known;
    return has_unknown(known);
}

bool component::has_unknown(std::vector<std::string>& known) const {
    if(is_expr()) {
        return expr_head().has_unknown(known) || expr_tail().has_unknown(known);
    } else if (is_lambda()) {
        known.push_back(lambda_arg().id_name());
        bool has = lambda_out().has_unknown(known);
        known.pop_back();
        return has;
    } else if (is_id()) {
        if(scope() == &prepkg::bound) {
            if (std::find(known.begin(), known.end(), id_name()) == known.end()) { // not in the input list
                return 1;
            } else { // is in the input list
                return 0;
            }
        } else if (scope() == &prepkg::global) {
            const component * value = global.get_value(id_name());
            if (value != nullptr) {
                return 0;
            } else {
                return 1;
            }
        } else {
            const component * value = scope()->get_value(id_name());
            if (value != nullptr) {
                return 0;
            } else {
                return 1;
            }
        }
    } else {
        throw std::logic_error("cannot call has_unknown on componenent of indeterminate type");
    }
}

bool component::lambda_unknown_before_arg() const {
    assert(is_lambda());
    return lambda_out().bound_in_output(lambda_arg().id_name()) && lambda_out().lambda_unknown_before_arg(lambda_arg().id_name()) != 2;
}

int component::lambda_unknown_before_arg(const std::string& argname) const {
    if (is_id()) {
        if(id_name() == argname)
            return 2;
        if(has_unknown())
            return 1;
        return 0;
    } else if (is_lambda()) {
        if(lambda_arg().id_name() == argname)
            return 0;
        return lambda_out().lambda_unknown_before_arg(argname);
    } else if (is_expr()) {
        return expr_head().lambda_unknown_before_arg(argname) ?: expr_tail().lambda_unknown_before_arg(argname);
    } else {
        throw std::logic_error("cannot call lambda_unknown_before_arg on component of indeterminate type");
    }
}

int component::evaluate_step() {
    if(is_expr()) {
        if(expr_head().evaluate_step()) {
            return 1;
        }
        if (!((expr_head().is_expr() || expr_head().is_id()) && expr_head().has_unknown()) &&
                !(expr_head().is_lambda() && !expr_head().lambda_unknown_before_arg())) { //otherwise value might not even be used
            if(expr_tail().evaluate_step())
                return 1;
        }
        if (expr_head().is_lambda()) {
            return evaluate_expression();
        }
        return 0;
    } else if (is_lambda()) {
        return lambda_out().evaluate_step();
    } else if (is_id()) {
        if(scope() == &prepkg::bound) {
            return 0;
        } else if (scope() == &prepkg::global) {
            const component * value = global.get_value(id_name());
            if (value != nullptr) {
                copy_preserve_parent(*value);
                return 1;
            } else {
                return 0;
            }
        } else {
            const component * value = scope()->get_value(id_name());
            if (value != nullptr) {
                copy_preserve_parent(*value);
                return 1;
            } else {
                return 0;
            }
        }
    } else {
        throw std::logic_error("cannot evaluate uninitialized expression");
    }
}

std::string component::to_string() const {
    std::string ret = "";

    if (is_lambda()) {
        const component * node = this;
        ret += "L";
        assert(lambda_arg().is_id());
        ret += lambda_arg().to_string();
        node = &lambda_out();
        while(node->is_lambda()) {
            assert(node->lambda_arg().is_id());
            ret += node->lambda_arg().to_string();
            node = &node->lambda_out();
        }
        ret += ".";
        assert(node->is_init());
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
#ifndef NDEBUG
        bool prime = false;
        for(size_t i = 0; i < id_name().size(); ++i) {
            assert(!is_name_break(id_name().at(i)));
            assert(id_name().at(i) >= '!');//not a control character or space
            assert(id_name().at(i) <= '~');//not DEL or extended ascii
            if(id_name().at(i) == '\'')
                prime = true;
            if(prime)
                assert(id_name().at(i) == '\'');
        }
#endif
        switch(base_name(id_name()).size()) {
        case 0:
            throw std::logic_error("identifier has name of length 0");
        case 1:
            if(id_name().at(0) != 'L') {
                ret += id_name();
                break;
            }
            //else fallthrough
        default:
            if(id_name().at(id_name().size() - 1) == '\'') {
                if(id_name().at(1) == '\'' && id_name().at(0) != 'L') {
                    ret += id_name();
                } else {
                    ret += "`";
                    ret += id_name();
                }
            } else {
                ret += "`";
                ret += id_name();
                ret += " ";
            }
            break;
        }
    } else {
        throw std::logic_error("component not a lambda, expression, or identifier");
    }


    for(size_t i = 0; i < ret.size();) {
        if(ret.at(i) == ' ' && ((i + 1) < ret.size() ? is_name_break(ret.at(i + 1)) : 0)) {
            ret.erase(i, 1);
        } else {
            ++i;
        }
    }

    return ret;
}



const component * package::get_value(std::string key) const {
    try { //holding out for c++20 map::contains
        return &values.at(key);
    } catch(std::out_of_range) {
        return nullptr;
    }
}
void package::add_value(std::string key, const component& to_add) {
    values[key] = to_add;
}

const component * global_package::get_value(std::string key) const {
    const component * ret = base.get_value(key);
    if (ret == nullptr) {
        for(const std::pair<const std::string,package>& p : packages) {
            ret = p.second.get_value(key);
            if(ret != nullptr) {
                return ret;
            }
        }
        return nullptr;
    } else {
        return ret;
    }
}

const component * global_package::get_value(std::string component_key, std::vector<std::string> package_keys) const {
    const component * ret;
    for(std::vector<std::string>::const_reverse_iterator pki = package_keys.crbegin(); pki != package_keys.crend(); ++pki) {
        const package * p = get_package(*pki);
        if(p == nullptr) {
            throw std::logic_error("attempted to access nonexistant package");
        }
        ret = p->get_value(component_key);
        if(ret != nullptr)
            return ret;
    }

    ret = base.get_value(component_key);
    if (ret == nullptr) {
        for(const std::pair<const std::string,package>& p : packages) {
            ret = p.second.get_value(component_key);
            if(ret != nullptr) {
                return ret;
            }
        }
        return nullptr;
    } else {
        return ret;
    }
}


void global_package::add_value(std::string key, const component& to_add) {        
    base.add_value(key, to_add);
}

package * global_package::add_package(std::string key) {
    if(key == "global")
        return &base;
    return &packages[key];
}

const package * global_package::get_package(std::string key) const {
    if(key == "global")
        return &base;
    try {
        return &packages.at(key);
    } catch (std::out_of_range) {
        return nullptr;
    }
}

package * global_package::get_package(std::string key) {
    if(key == "global")
        return &base;
    try {
        return &packages.at(key);
    } catch (std::out_of_range) {
        return nullptr;
    }
}

}


