#include "component.h"
#include "token.h"
#include <algorithm>

namespace lambda {

int component::replace_ids(std::string replace_name, const component& replace, package * replace_scope) {
    if (is_id()) {
        if (id_name() == replace_name && scope(replace_scope)) {
            copy_preserve_parent(replace);
            return 1;
        } else {
            return 0;
        }
    } else if (is_expr()) {
        return expr_head().replace_ids(replace_name, replace, replace_scope) + 
            expr_tail().replace_ids(replace_name, replace, replace_scope);
    } else if (is_lambda()) {
        if(replace_scope->is_bound() && lambda_arg().id_name() == replace_name) {
            return 0;
        } else {
            return lambda_out().replace_ids(replace_name, replace, replace_scope);
        }
    }
    throw std::logic_error("component not a lambda, expression, or identifier");
}

int component::evaluate_expression() {
    int ret = 0;
    if(expr_head().is_id() && !expr_head().is_bound()) {
        const component * id_value = nullptr;
        if(expr_head().is_global()) {
            id_value = global_scope()->get_value(expr_head().id_name());
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
            if(expr_tail().bound_from_above(node->lambda_arg().id_name())) {
                unsigned rev = 0;
                std::string base = base_name(node->lambda_arg().id_name());
                std::string alt = alt_name(base, rev);
                while(expr_tail().bound_from_above(alt) || node->lambda_out().bound_above_below(alt)) {
                    ++rev;
                    alt = alt_name(base, rev);
                }
                node->lambda_out().replace_ids(node->lambda_arg().id_name(), component().id(alt), node->global_scope()->get_bound());
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
    } else if (is_global()) {
        const component * value = global_scope()->get_value(id_name());
        if(value == nullptr) {
            return 0;
        } else {
            copy_preserve_parent(*value);
            return 1;
        }
    } else if (is_bound()) {
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
        if(is_bound()) {
            if (std::find(known.begin(), known.end(), id_name()) == known.end()) { // not in the input list
                return 1;
            } else { // is in the input list
                return 0;
            }
        } else if (is_global()) {
            const component * value = global_scope()->get_value(id_name());
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
    return lambda_out().bound_above_below(lambda_arg().id_name()) && lambda_out().lambda_unknown_before_arg(lambda_arg().id_name()) != 2;
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
        if(is_bound()) {
            return 0;
        } else if (is_global()) {
            const component * value = global_scope()->get_value(id_name());
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

std::vector<component*> component::find_steps() {
    std::vector<component*> ret;
    if(is_expr()) {
        if(expr_head().is_lambda()) {
            ret.push_back(this);
        }
        std::vector<component*> temp = expr_head().find_steps();
        ret.insert(ret.end(), temp.begin(), temp.end());
        temp = expr_tail().find_steps();
        ret.insert(ret.end(), temp.begin(), temp.end());
    } else if (is_lambda()) {
        std::vector<component*> temp = lambda_out().find_steps();
        ret.insert(ret.end(), temp.begin(), temp.end());
    } else if (is_id()) {
        if(is_global()) {
            if(global_scope()->get_value(id_name()) != nullptr)
                ret.push_back(this);            
        } else if (!is_bound()) {
            if(scope()->get_value(id_name()) != nullptr)
                ret.push_back(this);
        }
    } else {
        throw std::logic_error("cannot evaluate unitialized expression");
    }
    return ret;
}

bool component::match_bound(std::string myid, const component& comp, std::string compid) const {
    if(is_lambda()) {
        if(!comp.is_lambda()) {
            return false;
        }
        if(lambda_arg().id_name() == myid) {
            if(comp.lambda_out().bound_from_above(compid))
                return false;
            else
                return true;
        }
        if(comp.lambda_arg().id_name() == compid) {
            if(lambda_out().bound_from_above(myid))
                return false;
            else
                return true;
        }
        return lambda_out().match_bound(myid, comp.lambda_out(), compid);
    } else if (is_expr()) {
        if(!comp.is_expr()) {
            return false;
        }
        return expr_head().match_bound(myid,comp.expr_head(),compid) && expr_tail().match_bound(myid,comp.expr_tail(),compid);
    } else if (is_id()) {
        if(!comp.is_id()) {
            return false;
        }
        return true;
    } else {
        return false;
    }
}

bool component::lambda_arg_match(const component& comp) const {
    assert(is_lambda());
    if(!comp.is_lambda()) {
        return false;
    }
    return match_bound(lambda_arg().id_name(), comp, comp.lambda_arg().id_name());
}

bool component::equivalent(const component& comp) const {
    if(is_lambda()) {
        if(!comp.is_lambda()) {
            return false;
        }
        if(lambda_arg_match(comp)) {
            return lambda_out().equivalent(comp.lambda_out());
        } else {
            return false;
        }
    } else if (is_expr()) {
        if(!comp.is_expr()) {
            return false;
        }
        return expr_head().equivalent(comp.expr_head()) && expr_tail().equivalent(comp.expr_tail());
    } else if (is_id()) {
        if(!comp.is_id()) {
            return false;
        }
        if(!is_bound() || !comp.is_bound()) {
            if(scope() != comp.scope() || id_name() != comp.id_name())
                return false;
            else
                return true;
        } else { //let the lambda section determine how to interpret bound ids
            return true;
        }
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
        if(ret.at(i) == ' ' && ((i + 1) < ret.size() ? is_name_break(ret.at(i + 1)) : 1)) {
            ret.erase(i, 1);
        } else {
            ++i;
        }
    }

    return ret;
}

std::pair<std::string, std::vector<step_string_info>> component::step_string(const std::vector<component*>& steps) const {
    std::pair<std::string, std::vector<step_string_info>> ret;
    std::vector<component*> my_steps = steps;
    ret.second = step_string(ret.first, my_steps);
    return ret;
}

std::vector<step_string_info> component::step_string(std::string& out, std::vector<component*>& steps) const {
    std::vector<step_string_info> ret;

    if (is_lambda()) {
        const component * node = this;
        out += "L";
        assert(lambda_arg().is_id());
        out += lambda_arg().to_string();
        node = &lambda_out();
        while(node->is_lambda()) {
            assert(node->lambda_arg().is_id());
            node->lambda_arg().step_string(out, steps);
            node = &node->lambda_out();
        }
        out += ".";
        assert(node->is_init());
        node->step_string(out, steps);
    } else if (is_expr()) {
        std::vector<component*>::iterator pos = std::find(steps.begin(), steps.end(), this);

        if(pos != steps.end()) {
            step_string_info to_add;
            to_add.pos = *pos;
            steps.erase(pos);
            to_add.begin = out.size();
            if(expr_head().is_lambda()) {
                out += "(";
                std::vector<step_string_info> subret = expr_head().step_string(out, steps);
                out += ")";
                ret.insert(ret.end(), subret.begin(), subret.end());
            } else {
                std::vector<step_string_info> subret = expr_head().step_string(out, steps);
                ret.insert(ret.end(), subret.begin(), subret.end());
            }
            to_add.middle = out.size();
            if(expr_tail().is_lambda() || expr_tail().is_expr()) {
                out += "(";
                std::vector<step_string_info> subret = expr_tail().step_string(out, steps);
                out += ")";
                ret.insert(ret.end(), subret.begin(), subret.end());
            } else {
                std::vector<step_string_info> subret = expr_tail().step_string(out, steps);
                ret.insert(ret.end(), subret.begin(), subret.end());
            }
            to_add.end = out.size();
        } else {
            if(expr_head().is_lambda()) {
                out += "(";
                std::vector<step_string_info> subret = expr_head().step_string(out, steps);
                out += ")";
                ret.insert(ret.end(), subret.begin(), subret.end());
            } else {
                std::vector<step_string_info> subret = expr_head().step_string(out, steps);
                ret.insert(ret.end(), subret.begin(), subret.end());
            }
            if(expr_tail().is_lambda() || expr_tail().is_expr()) {
                out += "(";
                std::vector<step_string_info> subret = expr_tail().step_string(out, steps);
                out += ")";
                ret.insert(ret.end(), subret.begin(), subret.end());
            } else {
                std::vector<step_string_info> subret = expr_tail().step_string(out, steps);
                ret.insert(ret.end(), subret.begin(), subret.end());
            }
        }
    } else if (is_id()) {
        std::vector<component*>::iterator pos = std::find(steps.begin(), steps.end(), this);
        step_string_info to_add;
        if(pos != steps.end()) {
            to_add.pos = *pos;
            to_add.begin = out.size();
            to_add.middle = out.size();
        }
        switch(base_name(id_name()).size()) {
        case 0:
            throw std::logic_error("identifier has name of length 0");
        case 1:
            if(id_name().at(0) != 'L') {
                out += id_name();
                break;
            }
            //else fallthrough
        default:
            if(id_name().at(id_name().size() - 1) == '\'') {
                if(id_name().at(1) == '\'' && id_name().at(0) != 'L') {
                    out += id_name();
                } else {
                    out += "`";
                    out += id_name();
                }
            } else {
                out += "`";
                out += id_name();
                out += " ";
            }
            break;
        }
        if(pos != steps.end()) {
            to_add.end = out.size();
            steps.erase(pos);
        }
    } else {
        throw std::logic_error("component not a lambda, expression, or identifier");
    }

    for(size_t i = 0; i < out.size();) {
        if(out.at(i) == ' ' && ((i + 1) < out.size() ? is_name_break(out.at(i + 1)) : 1)) {
            for(step_string_info& ssi : ret) {
                if(ssi.begin > i)
                    --ssi.begin;
                if(ssi.middle > i)
                    --ssi.middle;
                if(ssi.end > i)
                    --ssi.end;
            }
            out.erase(i, 1);
        } else {
            ++i;
        }
    }
}




}


