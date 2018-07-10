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
    package base;
    std::map<std::string, package> packages;
    std::vector<std::string> open;
public:
    package * add_package(std::string key);
    package * get_package(std::string key);

    const component * get_value(std::string key);
    const component * get_value(std::string component_key, std::vector<std::string> package_keys);
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
    component& copy(const component& in) {
        nullable<component> head = in._head;//these are necessary to prevent loss of data if in is a child or this
        nullable<component> tail = in._tail;

        _parent = nullptr;
        _scope = in._scope;
        _name = in._name;
        _head = std::move(head);
        _tail = std::move(tail);
        if(!_head.null()) {
            _head.get().parent() = this;
        }
        if(!_tail.null()) {
            _head.get().parent() = this;
        }
        return *this;
    }
    component& steal(component&& in) {
        nullable<component> head = in._head;//these are necessary to prevent loss of data if in is a child or this
        nullable<component> tail = in._tail;

        _parent = nullptr;
        _scope = std::move(in._scope);
        _name = std::move(in._name);
        _head = std::move(head);
        _tail = std::move(tail);
        if(!_head.null()) {
            _head.get().parent() = this;
        }
        if(!_tail.null()) {
            _head.get().parent() = this;
        }

        assert(is_deep_alloc());
        return *this;
    }
    component() {
        _parent = nullptr;
        _scope = nullptr;
    }
    component(const component& in) {
        copy(in);
    }
    component(component&& in) {
        steal(std::move(in));
    }
    component& operator=(const component& in) { 
        return copy(in);
    }
    component& operator=(component&& in) {
        return steal(std::move(in));
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
        assert(is_deep_alloc());
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

    component& clear() {
        _head.nullify();
        _tail.nullify();
        _scope = nullptr;
        _name.nullify();
        
        return *this;
    }
    component& id(std::string name, package * pkg = &prepkg::bound) {
        _head.nullify();
        _tail.nullify();
        _scope = pkg;
        _name = name;

        return *this;
    }
    component& lambda(const component& arg, const component& out) {
        return lambda(std::move(component(arg)), std::move(component(out)));
    }
    component& lambda(component&& arg, const component& out) {
        return lambda(std::move(arg), std::move(component(out)));
    }
    component& lambda(const component& arg, component&& out) {
        return lambda(std::move(component(arg)), std::move(out));
    }
    component& lambda(component&& arg, component&& out) {
        component new_arg = std::move(arg);//these are necessary to prevent loss of data if arg or out is a child or this
        component new_out = std::move(out);

        //TODO verify that stealing a child works
        _scope = &prepkg::lambda;

        lambda_arg(std::move(new_arg));
        lambda_out(std::move(new_out));
        _name.nullify();

        lambda_arg().parent() = this;
        lambda_out().parent() = this;

        assert(is_deep_alloc());
        return *this;
    }
    component& expr(const component& head, const component& tail) {
        return expr(std::move(component(head)), std::move(component(tail)));
    }
    component& expr(component&& head, const component& tail) {
        return expr(std::move(head), std::move(component(tail)));
    }
    component& expr(const component& head, component&& tail) {
        return expr(std::move(component(head)), std::move(tail));
    }
    component& expr(component&& head, component&& tail) {
        component new_head = std::move(head);//these are necessary to prevent loss of data if head or tail is a child or this
        component new_tail = std::move(tail);

        _scope = &prepkg::expr;
        
        expr_head(std::move(new_head));
        expr_tail(std::move(new_tail));
        _name.nullify();

        expr_head().parent() = this;
        expr_tail().parent() = this;

        assert(is_deep_alloc());
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
            return 0;
        }
    }
    bool is_deep_alloc() {
        if(is_expr()) {
            if(_head.null() || _tail.null())
                return 0;
            return expr_head().is_deep_alloc() && expr_tail().is_deep_alloc();
        } else if(is_lambda()) {
            if(_head.null() || _tail.null())
                return 0;
            return lambda_arg().is_deep_alloc() && lambda_out().is_deep_alloc();
        } else if(is_id()) {
            return 1;
        } else {
            return 1;
        }
    }

    component& append(const component& to_add) {
        if(is_init()) {
            if (is_expr() && !expr_tail().is_init())
                expr_tail(to_add);
            else if (is_lambda())
                lambda_out().append(to_add);
            else
                expr(std::move(*this), to_add);
        } else {
            copy_preserve_parent(to_add);
        }
        return *this;
    }
    component& append(component&& to_add) {
        if(is_init()) {
            if (is_expr() && !expr_tail().is_init())
                expr_tail(std::move(to_add));
            else if (is_lambda())
                lambda_out().append(std::move(to_add));
            else
                expr(std::move(*this), std::move(to_add));
        } else {
            copy_preserve_parent(std::move(to_add));
        }
        return *this;
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
    static std::string alt_name(std::string base_name, unsigned rev) {
        std::string ret = base_name;
        for(unsigned i = 0; i < rev; ++i)
            ret += "'";
        return ret;
    }
    static std::string base_name(std::string name) {
        return name.substr(0, name.find_first_of('\''));
    }
    static unsigned base_rev(std::string name) {
        return name.size() - name.find_first_of('\'');
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
    bool lambda_has_arg(std::string check) {
        if(!is_lambda())
            return false;
        if(lambda_arg().id_name() == check)
            return true;
        return lambda_out().lambda_has_arg(check);
    }


    std::string to_string() const;
};













}
















#endif