#ifndef COMPONENT_H
#define COMPONENT_H

#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include "nullable.h"
#include <limits>

namespace lambda {

class package;
class component;

/**
 * Stores values of idenitifiers in `packages`. Identifiers that belong to a specific package
 * should store a pointer to that package. There are no nested packages.
 * 
 * Special packages are dummies that indicate a more general attribute (namely global, bound, lambda, expr)
 **/
class package {
private:
    std::map<std::string, component> values;
public:
    const component * get_value(std::string key);
    void add_value(std::string key, const component& to_add);
};

/**
 * Containes dummy packages whit special meanings
 **/
namespace prepkg {
    /**Indicates an identifier that corresponds to an argument to a lambda**/
    inline package bound;
    /**Indicates an identifier that does not belong to a specific package, but rather is globally accessable**/
    inline package global;
    /**Indicates a component that is a lambda expression. See component::lambda()**/
    inline package lambda;
    /**Indicates a component that is an expression. See component::expr()**/
    inline package expr;
}

class global_package {
private:
    std::map<std::string, component> values;
    std::map<std::string, package> packages;
    std::vector<std::string> open;
public:
    package * add_package(std::string key);
    package * get_package(std::string key);

    const component * get_value(std::string key);
    void add_value(std::string key, const component& to_add);
};

/**singleton used by components**/
inline global_package global;

class component {
private:
    component* _parent;
    package* _scope;
    
    nullable<std::string> _name;

    nullable<component> _head;
    nullable<component> _tail;

public:
    component() {
        _parent = nullptr;
        _scope = nullptr;
    }
    component(const component& copy) {
        _parent = nullptr;
        _scope = copy._scope;
        _name = copy._name;
        _head = copy._head;
        _tail = copy._tail;
        if(!_head.null()) {
            _head.get().parent() = this;
        }
        if(!_tail.null()) {
            _head.get().parent() = this;
        }
    }
    component(component&& steal) {
        nullable<component> head = std::move(steal._head);
        nullable<component> tail = std::move(steal._tail);

        _parent = nullptr;
        _scope = std::move(copy._scope);
        _name = std::move(copy._name);
        _head = head;
        _tail = tail;
        if(!_head.null()) {
            _head.get().parent() = this;
        }
        if(!_tail.null()) {
            _head.get().parent() = this;
        }
    }

    component& copy_preserve_parent(const component& copy) {
        component * temp_parent = parent();
        *this = copy;
        parent() = temp_parent;
        return *this;
    }
    component& copy_preserve_parent(component&& steal) {
        component * temp_parent = parent();
        *this = std::move(steal);
        parent() = temp_parent;
        return *this;
    }


    component*& parent() { return _parent; }
    component* base_parent() { 
        component * node = this;
        while(node->parent() != nullptr) {
            node = node->parent();
        }
        return node;
    }
    package*& scope() { return _scope; }
    const component* const parent() const { return _parent; }
    const component* base_parent() const { 
        const component * node = this;
        while(node->parent() != nullptr) {
            node = node->parent();
        }
        return node;
    }
    const package * const scope() const { return _scope; }

    component& expr_head() {
        assert(is_expr());
        return _head.get();
    }
    component& expr_tail() {
        assert(is_expr());
        return _tail.get();
    }
    const component& expr_head() const {
        assert(is_expr());
        return _head.get();
    }
    const component& expr_tail() const {
        assert(is_expr());
        return _tail.get();
    }
    component& expr_head(const component& copy) {
        assert(is_expr());
        _head.set(copy);
        _head.get().parent() = this;
        return _head.get();
    }
    component& expr_head(component&& copy) {
        assert(is_expr());
        _head.set(std::move(copy));
        _head.get().parent() = this;
        return _head.get();
    }
    component& expr_tail(const component& copy) {
        assert(is_expr());
        _tail.set(copy);
        _tail.get().parent() = this;
        return _tail.get();
    }
    component& expr_tail(component&& steal) {
        assert(is_expr());
        _tail.set(std::move(steal));
        _tail.get().parent() = this;
        return _tail.get();
    }

    std::string& id_name() {
        assert(is_id());
        return _name.get();
    }
    const std::string& id_name() const {
        assert(is_id());
        return _name.get();
    }

    component& lambda_arg() {
        assert(is_lambda());
        return _head.get();
    }
    component& lambda_out() {
        assert(is_lambda());
        return _tail.get();
    }
    const component& lambda_arg() const {
        assert(is_lambda());
        return _head.get();
    }
    const component& lambda_out() const {
        assert(is_lambda());
        return _tail.get();
    }
    component& lambda_arg(const component& copy) {
        assert(is_lambda());
        _head.set(copy);
        _head.get().parent() = this;
        return _head.get();
    }
    component& lambda_arg(component&& steal) {
        assert(is_lambda());
        _head.set(std::move(steal));
        _head.get().parent() = this;
        return _head.get();
    }
    component& lambda_out(const component& copy) {
        assert(is_lambda());
        _tail.set(copy);
        _tail.get().parent() = this;
        return _tail.get();
    }
    component& lambda_out(component&& steal) {
        assert(is_lambda());
        _tail.set(std::move(steal));
        _tail.get().parent() = this;
        return _tail.get();
    }

    component& id(std::string name, package * pkg = &prepkg::bound) {
        _head.nullify();
        _tail.nullify();
        _scope = pkg;
        _name = name;
        _parent = nullptr;

        return *this;
    }
    component& lambda(component arg, component out) {
        _scope = &prepkg::lambda;
        lambda_arg(arg);
        lambda_out(out);
        _name.nullify();

        lambda_arg().parent() = this;
        lambda_out().parent() = this;
        _parent = nullptr;

        return *this;
    }
    component& expr(component head, component tail) {
        _scope = &prepkg::expr;
        expr_head(head);
        expr_tail(tail);
        _name.nullify();

        expr_head().parent() = this;
        expr_tail().parent() = this;
        _parent = nullptr;

        return *this;
    }

    bool is_id() const { return _scope != &prepkg::expr && _scope != &prepkg::lambda && _scope != nullptr; }
    bool is_expr() const { return _scope == &prepkg::expr; }
    bool is_lambda() const { return _scope == &prepkg::lambda; }
    bool is_init() const { return _scope != nullptr; }
    bool is_deep_init() const { 
        if(is_expr()) {
            return expr_head().is_deep_init() && expr_tail().is_deep_init();
        } else if(is_lambda()) {
            return lambda_arg().is_deep_init() && lambda_out().is_deep_init();
        } else if(is_id()) {
            return 1;
        } else {
            return false;
        }
    }

    component& append(component to_add) {
        if(is_init) {
            expr(component(std::move(*this)), to_add);
        } else {
            copy_preserve_parent(to_add);
        }
        *this;
    }
    
    int evaluate_step();
    int evaluate();
    int evaluate_expression();
    int simplify_step();
    int simplify();

    int replace_ids(std::string replace_name, const component& replace, package * replace_scope = &prepkg::bound);

    std::string first_name_not_in_ancestors(std::string base_name) const {
        if(!bound_in_ancestor(base_name))
            return base_name;
        for(unsigned i = 0; i < std::numeric_limits<unsigned>::max(); ++i) {
            std::string name = alt_name(base_name, i);
            if(!bound_in_ancestor(name))
                return name;            
        }
        return base_name + "@OVERFLOW";
    }
    static std::string alt_name(std::string name, unsigned rev) {
        if(rev == 0) {
            return name;
        } else {
            return name + "@" + std::to_string(rev);
        }
    }
    static std::string base_name(std::string name) {
        return name.substr(0, name.find_first_of('@'));
    }
    static unsigned base_rev(std::string name) {
        try {
            std::string::size_type index = name.find_first_of('@');
            if(index == name.size() || index == name.size() - 1)
                return 0;
            return stoul(name.substr(name.find_first_of('@') + 1));
        } catch (std::exception) {
            throw std::logic_error("alt name invalid");
        }
    }
    bool bound_in_argument(const std::string& check) const {
        if(is_lambda()) {
            if(lambda_arg().id_name() == check) {
                return false;
            } else {
                return lambda_out().bound_in_argument(check);
            }
        } else if (is_expr()) {
            return expr_head().bound_in_argument(check)
                || expr_tail().bound_in_argument(check);
        } else if (is_id()) {
            if(id_name() == check && scope() == &prepkg::bound) {
                return true;
            } else {
                return false;
            }
        } else {
            throw std::logic_error("component not a lambda, expression, or identifier");
        }
    }
    bool bound_in_output(const std::string& check) const {
        if(is_lambda()) {
            if(lambda_arg().id_name() == check) {
                return true;
            } else {
                return lambda_out().bound_in_output(check);
            }
        } else if (is_expr()) {
            return expr_head().bound_in_output(check)
                || expr_tail().bound_in_output(check);
        } else if (is_id()) {
            if(id_name() == check && scope() == &prepkg::bound) {
                return true;
            } else {
                return false;
            }
        } else {
            throw std::logic_error("component not a lambda, expression, or identifier");
        }
    }
    bool bound_in_ancestor(std::string check) const {
        const component * node = this;
        while(node != nullptr) {
            if(node->is_lambda() && node->lambda_arg().id_name() == check)
                return 1;
            node = node->parent();
        }
        return 0;
    }


    std::string to_string() const;
};













}
















#endif