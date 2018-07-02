#include "compile.h"
#include <assert.h>
#include <algorithm>

namespace lambda {


int replace_bound_in_expression(std::string name, expression& expr, const component& replace_with) {
    for(size_t index = 0; index < expr.sub.size(); ++index) {
        switch(expr.sub[index].type) {
        case component_type::EXPRESSION:
            replace_bound_in_expression(name,expr.sub[index].value.e,replace_with);
            break;
        case component_type::LAMBDA:
            if(std::find_if(expr.sub[index].value.l.arguments.begin(), 
                    expr.sub[index].value.l.arguments.begin(),
                    [&name](identifier i)->bool{ return i.name == name; }) == expr.sub[index].value.l.arguments.end()) {
                replace_bound_in_expression(name,expr.sub[index].value.l.output,replace_with);
            }
            break;
        case component_type::IDENTIFIER:
            if(expr.sub[index].value.i.scope == &bound) {
                expr.sub[index] = replace_with;
                expr.sub[index].parent(&expr);
            }
            break;
        default:
            throw std::logic_error("cannot evaluate component of indeterminate type");
        }
    }
}

int apply_step(component& function, const component& argument) {
    lambda& func = function.value.l;

    switch(function.type) {
    case component_type::EXPRESSION:
        return 0;
    case component_type::IDENTIFIER:
        if(function.value.i.value.null()) {
            return 0;
        } else {
            component * parent = function.parent;
            function = (lambda)function.value.i.value;
            function.parent = parent;
        }
        break;
    case component_type::LAMBDA:
        break;
    default:
        throw std::logic_error("cannot apply component of indeterminate type as function");
    }

    assert(function.type == component_type::LAMBDA);

    if(func.arguments.size() == 0) {
        throw std::logic_error("cannot apply lambda with zero arguments");
    } else {
        std::string argument_name = func.arguments[0];
        replace_bound_in_expression(argument_name, func.output, argument);
        func.arguments.pop_front();
        if(func.arguments.size() == 0) {
            component * parent = function.parent;
            function = function.value.l.output;
            function.parent = parent;
        }
    }

    return 0;

}

int evaluate_step_expression(component& arg) {
    expression& expr = arg.value.e;

    if(expr.sub.size() == 0) {
        throw std::logic_error("cannot evaluate empty expression");
    }

    for(size_t i = 0; i < expr.sub.size(); ++i) {
        if(evaluate_step(expr.sub[i]))
            return 1;
    }

    if(expr.sub.size() == 1) {
        size_t index;
        if(expr.parent->type == component_type::EXPRESSION) {
            component * parent = expr.parent;
            index = parent->value.e.find_sub(expr);
            if(index == -1)
                throw std::logic_error("expression's parent does not contain expression");
            parent->value.e.sub[index] = expr.sub[0];
            parent->value.e.sub[index].parent(parent);
            return 1;
        } else if (expr.sub[0].type == component_type::EXPRESSION) {
            //if parent is lambda, setting expr will set lambda.out
            //otherwise, only arg will have changed
            component * parent = expr.parent;
            expr = expr.sub[0].value.e;
            expr.parent = parent;
        } else {
            return 0;
        }
    } else /*size > 1*/ {
        for(size_t i = 0; i < expr.sub.size() - 1; ++i) {
            if(apply_step(expr.sub[0], expr.sub[1])) {
                if(expr.sub[0]
                return 1;
            }
        }
        return 0;
    }
}

int evaluate_step_lambda(component arg) {
    lambda& lamb = arg.value.l;
    
    return evaluate_step_expression(lamb.output);
}

int evaluate_step(component& arg) {
    switch(arg.type) {
    case component_type::EXPRESSION:
        return evaluate_step_expression(arg);
    case component_type::LAMBDA:
        return evaluate_step_lambda(arg);
    case component_type::IDENTIFIER:
        return 0;
    default:
        throw std::logic_error("cannot evaluate component of indeterminate type");
    }
}

int evaluate(component& arg) {
    int iterations = 0;
    while(evaluate_step(arg))
        ++iterations;
    return iterations;
}


}