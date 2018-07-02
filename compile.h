#include "token.h"
#include <optional>
#include <vector>
#include <string>
#include "nullable.h"
#include <map>

namespace lambda {

struct identifier;
struct lambda;
struct package;
struct expression;
struct component;

enum class component_type {
    EXPRESSION,
    LAMBDA,
    IDENTIFIER,
    NONE
};

struct identifier {
    component * parent;
    package * scope;
    std::string name;
    nullable<lambda> value;

    identifier& operator=(const identifier& copy);
};

struct package {
    std::string name;
    std::map<std::string, identifier *> contents;

    package();
} global, bound, free;

struct expression {
    component * parent;
    std::vector<component> sub;
    
    expression& operator=(const expression& copy);

    int find_sub(const component& which);
};

struct lambda {
    component * parent;
    std::vector<std::string> arguments;
    expression output;

    lambda& operator=(const lambda& copy);

};

struct component {
    component_type type;
    union U {
        expression e;
        lambda l;
        identifier i;
        U() {}
        U(const U&) { throw std::logic_error("component::U copy constructor should not be called"); }
        ~U() { }
        void operator=(const U&) { throw std::logic_error("component::U operator= should not be called");  }
    } value;

    component() { type = component_type::NONE; }
    component(const component& copy) {
        switch(copy.type) {
        case component_type::EXPRESSION:
            type = component_type::EXPRESSION;
            value.e = copy.value.e;
            break;
        case component_type::LAMBDA:
            type = component_type::LAMBDA;
            value.l = copy.value.l;
            break;
        case component_type::IDENTIFIER:
            type = component_type::IDENTIFIER;
            value.i = copy.value.i;
            break;
        default:
            type = component_type::NONE;
            break;
        }
    }

    ~component() {
        switch(type) {
        case component_type::EXPRESSION:
            value.e.~expression();
            break;
        case component_type::LAMBDA:
            value.l.~lambda();
            break;
        case component_type::IDENTIFIER:
            value.i.~identifier();
            break;
        default:
            break;
        }
    }

    component& operator=(component set) {
        this->~component();
        switch(set.type) {
        case component_type::EXPRESSION:
            type = component_type::EXPRESSION;
            value.e = set.value.e;
            break;
        case component_type::LAMBDA:
            type = component_type::LAMBDA;
            value.l = set.value.l;
            break;
        case component_type::IDENTIFIER:
            type = component_type::IDENTIFIER;
            value.i = set.value.i;
            break;
        default:
            type = component_type::NONE;
            break;
        }
        return *this;
    }
    
    component * parent() {
        switch(type) {
        case component_type::EXPRESSION:
            return value.e.parent;
            break;
        case component_type::LAMBDA:
            return value.l.parent;
            break;
        case component_type::IDENTIFIER:
            return value.i.parent;
            break;
        default:
            return (component*)nullptr;
            break;
        }
    }

    void parent(component * set) {
        switch(type) {
        case component_type::EXPRESSION:
            value.e.parent = set;
            break;
        case component_type::LAMBDA:
            value.l.parent = set;
            break;
        case component_type::IDENTIFIER:
            value.i.parent = set;
            break;
        default:
            throw std::logic_error("attempting to set parent of void component");
            break;
        }
    }
};

inline int expression::find_sub(const component& which) {
    switch(which.type) {
    case component_type::EXPRESSION:
        for(size_t i = 0; i < sub.size(); ++i) {
            if(sub[i].type == component_type::EXPRESSION && &(sub[i].value.e) == &(which.value.e)) {
                return i;
            }
        }
        return -1;
    case component_type::LAMBDA:
        for(size_t i = 0; i < sub.size(); ++i) {
            if(sub[i].type == component_type::LAMBDA && &(sub[i].value.l) == &(which.value.l)) {
                return i;
            }
        }
        return -1;
    case component_type::IDENTIFIER:
        for(size_t i = 0; i < sub.size(); ++i) {
            if(sub[i].type == component_type::IDENTIFIER && &(sub[i].value.i) == &(which.value.i)) {
                return i;
            }
        }
        return -1;
    default:
        for(size_t i = 0; i < sub.size(); ++i)
            if(sub[i].type == component_type::NONE)
                return i;
        return -1;
    }
}




int evaluate_step(component&);
int evaluate(component&);

int expand_step(component&);
int expand(component&);

int apply_step(component& function, const component& argument);

expression& expression::operator=(const expression& copy) {
    parent = copy.parent;
    sub = copy.sub;
}

lambda& lambda::operator=(const lambda& copy) {
    parent = copy.parent;
    arguments = copy.arguments;
    output = copy.output;
}

identifier& identifier::operator=(const identifier& copy) {
    parent = copy.parent;
    name = copy.name;
    value = copy.value;
    scope = copy.scope;
}

}