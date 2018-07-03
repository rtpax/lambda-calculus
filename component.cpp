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
        component * node = &expr_head();
        while(node->is_lambda()) {
            if(expr_tail().bound_in_children(node->lambda_arg().id_name())) {
                unsigned rev = 0;
                std::string base = base_name(node->lambda_arg().id_name());
                std::string alt = alt_name(base, rev);
                bool sub_lambda_has_id = 0;
                while(expr_tail().bound_in_children(alt) && !sub_lambda_has_id) {
                    sub_lambda_has_id = 0;
                    ++rev;
                    alt = alt_name(base, rev);
                    component * down_node = &node->lambda_out();
                    while(down_node->is_lambda()) {
                        if(alt == down_node->lambda_arg().id_name())
                            sub_lambda_has_id = 1;
                        down_node = &down_node->lambda_out();
                    }
                    component * up_node = node->parent();
                    while(up_node != nullptr && up_node->is_lambda()) {
                        if(alt == up_node->lambda_arg().id_name())
                            sub_lambda_has_id = 1;
                        up_node = up_node->parent();
                    }
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


using namespace lambda;


int main() {
    component a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z;

    global.add_value("S",
        d.lambda(e.id("w"),
            a.lambda(b.id("y"),
                c.lambda(d.id("x"),
                    e.expr(f.id("y"),
                        g.expr(h.expr(i.id("w"),j.id("y")),k.id("x")))))));
    
    global.add_value("0",
        a.lambda(b.id("s"),
            c.lambda(a.id("z"),
                b.id("z"))));
    
    global.add_value("1",
        a.expr(b.id("S",&prepkg::global),
            c.id("0",&prepkg::global)));

    global.add_value("2",
        a.expr(b.id("S",&prepkg::global),
            c.id("1",&prepkg::global)));

    global.add_value("3",
        a.expr(b.id("S",&prepkg::global),
            c.id("2",&prepkg::global)));

    global.add_value("4",
        a.expr(b.id("S",&prepkg::global),
            c.id("3",&prepkg::global)));

    a.lambda(
        b.id("x"),
        c.lambda(
            d.id("y"),
            e.expr(
                f.expr(
                    g.id("x"),
                    h.id("y")),
                i.expr(
                    j.id("y"),
                    k.id("x")))));

    z.expr(
        a,
        b.lambda(
            c.id("a"),
            d.lambda(
                e.id("b"),
                f.id("b"))));

    bool stepped = 1;
    int max_iter = 100;
    while(stepped && max_iter > 0) {
        std::cout << z.to_string() << "\n";
        stepped = z.evaluate_step();
        if(!stepped)
            stepped = z.simplify_step();
        --max_iter;
    }

    z.expr(a.expr(b.id("3",&prepkg::global),c.id("S",&prepkg::global)),d.id("0",&prepkg::global));    

    std::cout << "\n";
    stepped = 1;
    max_iter = 100;
    while(stepped && max_iter > 0) {
        std::cout << z.to_string() << "\n";
        stepped = z.evaluate_step();
        if(!stepped)
            stepped = z.simplify_step();
        --max_iter;
    }

    z.id("4",&prepkg::global);

    std::cout << "\n";
    stepped = 1;
    max_iter = 100;
    while(stepped && max_iter > 0) {
        std::cout << z.to_string() << "\n";
        stepped = z.evaluate_step();
        if(!stepped)
            stepped = z.simplify_step();
        --max_iter;
    }

    z.lambda(
        component().id("y"),
        component().lambda(
            component().id("x"),
            component().expr(
                component().id("y"),
                component().expr(
                    component().lambda(
                        component().id("y"),
                        component().lambda(
                            component().id("x"),
                            component().expr(
                                component().id("x"),
                                component().id("y")
                            )
                        )
                    ),
                    component().id("x")
                )
            )
        )
    );

    std::cout << "\n";
    stepped = 1;
    max_iter = 100;
    while(stepped && max_iter > 0) {
        std::cout << z.to_string() << "\n";
        stepped = z.evaluate_step();
        if(!stepped)
            stepped = z.simplify_step();
        --max_iter;
    }

}