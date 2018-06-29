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
struct ref_component;

enum class component_type {
    EXPRESSION,
    LAMBDA,
    IDENTIFIER,
    NONE
};

struct ref_component {
    component_type type;
    union U {
        expression* e;
        lambda* l;
        identifier* i;
        U() {}
        U(const U&) { throw std::logic_error("ref_component::U copy constructor should not be called"); }
        ~U() {}
        void operator=(const U&) { throw std::logic_error("ref_component::U operator= should not be called"); }
    } value;

    ref_component() { value.e = nullptr; type = component_type::NONE; }
    ref_component(expression& e) { value.e = &e; type = component_type::EXPRESSION; }
    ref_component(lambda& l) { value.l = &l; type = component_type::LAMBDA; }
    ref_component(identifier& i) { value.i = &i; type = component_type::IDENTIFIER; }
    ref_component(component&);

    ref_component parent();
    void parent(ref_component set);
};

struct const_ref_component {
    component_type type;
    union U {
        const expression* e;
        const lambda* l;
        const identifier* i;
        U() {}
        U(const U&) { throw std::logic_error("const_ref_component::U copy constructor should not be called"); }
        ~U() {}
        void operator=(const U&) { throw std::logic_error("const_ref_component::U operator= should not be called"); }
    } value;

    const_ref_component() { value.e = nullptr; type = component_type::NONE; }
    const_ref_component(const expression& e) { value.e = &e; type = component_type::EXPRESSION; }
    const_ref_component(const lambda& l) { value.l = &l; type = component_type::LAMBDA; }
    const_ref_component(const identifier& i) { value.i = &i; type = component_type::IDENTIFIER; }
    const_ref_component(const component&);
    const_ref_component(const ref_component&);

    const_ref_component parent();
};

struct identifier {
    ref_component parent;
    package * scope;
    std::string name;
    nullable<lambda> value;

    identifier() : name("") { }
};

struct package {
    std::string name;
    std::map<std::string, identifier *> contents;

    package();
} global, bound, free;

struct expression {
    ref_component parent;
    std::vector<component> sub;

    int find_sub(const_ref_component which);
    expression() {}
};

struct lambda {
    ref_component parent;
    std::vector<std::string> arguments;
    expression output;

    lambda() {}
};

struct component {
    component_type type;
    union U {
        expression e;
        lambda l;
        identifier i;
        U() {}
        U(const U&) { throw std::logic_error("component::U copy constructor should not be called"); }
        ~U() {}
        void operator=(const U&) { throw std::logic_error("component::U operator= should not be called");  }
    } value;

    component() { type = component_type::NONE; }
    component(const_ref_component copy) {
        switch(copy.type) {
        case component_type::EXPRESSION:
            type = component_type::EXPRESSION;
            value.e = *(copy.value.e);
            break;
        case component_type::LAMBDA:
            type = component_type::LAMBDA;
            value.l = *(copy.value.l);
            break;
        case component_type::IDENTIFIER:
            type = component_type::IDENTIFIER;
            value.i = *(copy.value.i);
            break;
        default:
            type = component_type::NONE;
            break;
        }
    }
    
    ref_component parent() {
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
            return ref_component();
            break;
        }
    }

    void parent(ref_component set) {
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

inline ref_component::ref_component(component& c) {
    switch(c.type) {
    case component_type::EXPRESSION:
        ref_component(c.value.e);
        break;
    case component_type::LAMBDA:
        ref_component(c.value.l);
        break;
    case component_type::IDENTIFIER:
        ref_component(c.value.i);
        break;
    default:
        ref_component(); 
        break;
    }
}

inline ref_component ref_component::parent() {
    switch(type) {
    case component_type::EXPRESSION:
        return value.e->parent;
        break;
    case component_type::LAMBDA:
        return value.l->parent;
        break;
    case component_type::IDENTIFIER:
        return value.i->parent;
        break;
    default:
        return ref_component();
        break;
    }
}

inline void ref_component::parent(ref_component set) {
    switch(type) {
    case component_type::EXPRESSION:
        value.e->parent = set;
        break;
    case component_type::LAMBDA:
        value.l->parent = set;
        break;
    case component_type::IDENTIFIER:
        value.i->parent = set;
        break;
    default:
        throw std::logic_error("attempting to set parent of void component");
        break;
    }
}

inline const_ref_component::const_ref_component(const ref_component& c) {
    switch(c.type) {
    case component_type::EXPRESSION:
        const_ref_component(*c.value.e);
        break;
    case component_type::LAMBDA:
        const_ref_component(*c.value.l);
        break;
    case component_type::IDENTIFIER:
        const_ref_component(*c.value.i);
        break;
    default:
        const_ref_component(); 
        break;
    }
}

inline const_ref_component::const_ref_component(const component& c) {
    switch(c.type) {
    case component_type::EXPRESSION:
        const_ref_component(c.value.e);
        break;
    case component_type::LAMBDA:
        const_ref_component(c.value.l);
        break;
    case component_type::IDENTIFIER:
        const_ref_component(c.value.i);
        break;
    default:
        const_ref_component(); 
        break;
    }
}

inline const_ref_component const_ref_component::parent() {
    switch(type) {
    case component_type::EXPRESSION:
        return value.e->parent;
        break;
    case component_type::LAMBDA:
        return value.l->parent;
        break;
    case component_type::IDENTIFIER:
        return value.i->parent;
        break;
    default:
        return const_ref_component();
        break;
    }
}

inline int expression::find_sub(const_ref_component which) {
    switch(which.type) {
    case component_type::EXPRESSION:
        for(size_t i = 0; i < sub.size(); ++i) {
            if(sub[i].type == component_type::EXPRESSION && &(sub[i].value.e) == (which.value.e)) {
                return i;
            }
        }
        return -1;
    case component_type::LAMBDA:
        for(size_t i = 0; i < sub.size(); ++i) {
            if(sub[i].type == component_type::LAMBDA && &(sub[i].value.l) == (which.value.l)) {
                return i;
            }
        }
        return -1;
    case component_type::IDENTIFIER:
        for(size_t i = 0; i < sub.size(); ++i) {
            if(sub[i].type == component_type::IDENTIFIER && &(sub[i].value.i) == (which.value.i)) {
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




int evaluate_step(ref_component);
int evaluate(ref_component);

int expand_step(ref_component);
int expand(ref_component);

int apply_step(component& output, const_ref_component function, const_ref_component argument);


}