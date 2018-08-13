#ifndef LAMBDA_COMPONENT_H
#define LAMBDA_COMPONENT_H

#include <cassert>
#include <string>
#include <vector>
#include "nullable.h"
#include "package.h"
#include <limits>

namespace lambda {


struct step_string_info {
    component * pos;
    int begin;
    int middle;
    int end;
};

class component_type_error : public std::logic_error {
    component_type_error(const std::string& message) : std::logic_error("component_type_error: " + message) {}
    component_type_error(package::category expected, package::category actual) : 
        std::logic_error("component_type_error: expected " + package::category_string(expected) + ", recieved " + package::category_string(actual)) {}
};
class component_error : public std::logic_error {
    component_error(const std::string& message) : std::logic_error(message) {}
};

/**
 * stores data for lambda expressions and conatains functions for computation
 * 
 * a component may be of three types
 *   - lambda
 *   - expr
 *   - id
 * 
 * when using methods prefixed with these names, they are specific to these types
 * and will result in an error if of the prefixed type. The only exception to this
 * are the functions `lambda()`, `expr()`, and `id()` which change the type
 *  
 * 
 * 
 **/
class component {
private:
    component* _parent;
    package* _scope;
    
    nullable<std::string> _name;

    nullable<component> _head;
    nullable<component> _tail;

public:
    /**
     * this becomes an identical copy of `in`. This includes type, parent, and children.
     * 
     * note that children values are copied, not addresses
     **/
    component& copy(const component& in) {
        nullable<component> head = in._head;//these are necessary to prevent loss of data if in a child or this
        nullable<component> tail = in._tail;

        _parent = nullptr;
        _scope = in._scope;
        _name = in._name;
        _head = std::move(head);
        _tail = std::move(tail);
        if(!_head.null()) {
            _head.get().parent(this);
        }
        if(!_tail.null()) {
            _head.get().parent(this);
        }
        return *this;
    }
    /**
     * Destructively copies `in`. The children of `in` will be nullified and be inaccessable after this call.
     * This will have the same parent and scope as `in`
     **/
    component& steal(component&& in) {
        nullable<component> head = in._head;//these are necessary to prevent loss of data if in a child or this
        nullable<component> tail = in._tail;

        _parent = nullptr;
        _scope = std::move(in._scope);
        _name = std::move(in._name);
        _head = std::move(head);
        _tail = std::move(tail);
        if(!_head.null()) {
            _head.get().parent(this);
        }
        if(!_tail.null()) {
            _head.get().parent(this);
        }

        assert(is_deep_alloc());
        return *this;
    }
    /**
     * Creates an unitialized component (ie is_init() will evaluate to false) in the default package space
     * 
     * Use of this is not recommended. It is generally better specify the package space you wish to use to
     * allow multiple lambda calculus programs run concurrently without conflict
     **/
    component() {
        _parent = nullptr;
        _scope = default_space.get_blank();
    }
    /**
     * creates an unitialized component (ie is_init() is false) in the specified package space
     **/
    component(global_package * package_space) {
    }
    /**
     * copy constructor
     * 
     * equivalent to `component foo; foo.copy(in);`
     **/
    component(const component& in) {
        copy(in);
    }
    /**
     * move constructor
     * 
     * equivalent to `component foo; foo.steal(in);`
     **/
    component(component&& in) {
        steal(std::move(in));
    }
    /**
     * equivalent to component::copy
     **/
    component& operator=(const component& in) { 
        return copy(in);
    }
    /**
     * equivalent to component::steal
     **/
    component& operator=(component&& in) {
        return steal(std::move(in));
    }

    /**
     * creates a copy of `copy` but does not modify parent and keeps the scope in the original package space
     **/
    component& copy_preserve_parent(const component& copy) {
        return copy_preserve_parent(std::move(component(copy)));
    }
    /**
     * move copies `steal` but does not modify parent and keeps the scope in the original package space
     **/
    component& copy_preserve_parent(component&& steal) {
        component * temp_parent = parent();
        package * temp_scope = scope();
        *this = std::move(steal);
        parent(temp_parent);
        scope(scope()->equivalent_in(temp_scope->package_space()));
        normalize_child_scope();
        return *this;
    }
    /**
     * returns a pointer to the parent component, or nullptr if it has no parent
     **/
    component* parent() { return _parent; }
    /**
     * returns a const pointer to the parent component, or nullptr if it has no parent
     **/
    const component* parent() const { return _parent; }
    /**
     * sets and the parent component to `replace` and returns
     * 
     * set to nullptr if there is no parent
     **/
    component* parent(component* replace) { return _parent = replace; }
    /**
     * recursively searches for the topmost parent (which may simply be this) and returns a pointer to it.
     **/
    component* base_parent() { return const_cast<component*>(const_cast<const component*>(this)->base_parent()); }
    /**
     * recursively searches for the topmost parent (which may simply be this) and returns a const pointer to it.
     **/
    const component* base_parent() const { 
        const component * node = this;
        while(node->parent() != nullptr) {
            node = node->parent();
        }
        return node;
    }

    global_package* global_scope() const { return _scope->package_space(); }
    global_package* global_scope(global_package* replace) { _scope = _scope->equivalent_in(replace); return global_scope(); }
    /**
     * Returns the package that determines the scope of this component. See lambda::package for details
     **/
    package* scope() const { return _scope; }
    /**
     * sets the scope to the specied package and returns it
     * 
     * note that this does not force consistency with the parent/previous value (use set_equivalent_scope for that)
     **/
    package* scope(package * replace) { return _scope = replace; }
    /**
     * Sets the scope to a scope of the corresponding category within the current package space
     * 
     * Input of PACKAGE is illegal (because it may refer to multiple packages).
     * To get an input of type PACKAGE, use `scope()->package_space()->get_package(<package-name>)`
     **/
    package* scope(package::category replace) { return _scope = _scope->package_space()->get_by_category(replace); }
    /**
     * Sets the scope to an equivalent package within the current package space.
     * See package::equivalent_in() for more details
     **/
    package* set_equivalent_scope(package* replace) { return _scope = _scope->equivalent_in(replace->package_space()); }
    /**
     * changes the scope to an equivalent scope within the parents package space.
     * parents and children should always be within the same namespace before performing computations.
     **/
    package* normalize_scope_to_parent() { 
        if(parent() == nullptr) {
            return scope();
        } else {
            return scope(scope()->equivalent_in(parent()->scope()->package_space()));
        }
    }
    /**
     * recursively changes the scope of the descendant to be within the same package space as this
     **/
    void normalize_child_scope() {
        if(is_lambda()) {
            lambda_arg().normalize_scope_to_parent();
            lambda_arg().normalize_child_scope();
            lambda_out().normalize_scope_to_parent();
            lambda_out().normalize_child_scope();
        } else if(is_expr()) {
            expr_head().normalize_scope_to_parent();
            expr_head().normalize_child_scope();
            expr_tail().normalize_scope_to_parent();
            expr_tail().normalize_child_scope();
        }
    }

    /**
     * returns the head of the expression, ie the part that appears first.
     * 
     * do not set this with `operator=`, use `copy_preserve_parent(arg)` or `expr_head(arg)` to set
     **/
    component& expr_head() {
        assert(is_expr());
        return _head.get();
    }
    /**
     * returns the head of the expression, ie the part that appears second.
     * 
     * do not set this with `operator=`, use `copy_preserve_parent(arg)` or `expr_tail(arg)` to set
     **/
    component& expr_tail() {
        assert(is_expr());
        return _tail.get();
    }
    /**
     * returns a const refernce to the expression head
     **/
    const component& expr_head() const {
        assert(is_expr());
        return _head.get();
    }
    /**
     * returns a const reference to the expression tail
     **/
    const component& expr_tail() const {
        assert(is_expr());
        return _tail.get();
    }
    /**
     * sets the expression head, but modifies the parent and package space to match the parent
     **/
    component& expr_head(const component& copy) {
        assert(is_expr());
        _head.set(copy);
        _head.get().parent(this);
        _head.get().normalize_scope_to_parent();
        _head.get().normalize_child_scope();
        return _head.get();
    }
    /**
     * sets the expression head, but modifies the parent and package space to match the parent
     **/
    component& expr_head(component&& copy) {
        assert(is_expr());
        _head.set(std::move(copy));
        _head.get().parent(this);
        _head.get().normalize_scope_to_parent();
        _head.get().normalize_child_scope();
        return _head.get();
    }
    component& expr_tail(const component& copy) {
        assert(is_expr());
        _tail.set(copy);
        _tail.get().parent(this);
        _tail.get().normalize_scope_to_parent();
        _tail.get().normalize_child_scope();
        return _tail.get();
    }
    component& expr_tail(component&& steal) {
        assert(is_expr());
        _tail.set(std::move(steal));
        _tail.get().parent(this);
        _tail.get().normalize_scope_to_parent();
        _tail.get().normalize_child_scope();
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
        _head.get().parent(this);
        _head.get().normalize_scope_to_parent();
        _head.get().normalize_child_scope();
        return _head.get();
    }
    component& lambda_arg(component&& steal) {
        assert(is_lambda());
        _head.set(std::move(steal));
        _head.get().parent(this);
        _head.get().normalize_scope_to_parent();
        _head.get().normalize_child_scope();
        return _head.get();
    }
    component& lambda_out(const component& copy) {
        assert(is_lambda());
        _tail.set(copy);
        _tail.get().parent(this);
        _tail.get().normalize_scope_to_parent();
        _tail.get().normalize_child_scope();
        return _tail.get();
    }
    component& lambda_out(component&& steal) {
        assert(is_lambda());
        _tail.set(std::move(steal));
        _tail.get().parent(this);
        _tail.get().normalize_scope_to_parent();
        _tail.get().normalize_child_scope();
        return _tail.get();
    }

    component& clear() {
        _head.nullify();
        _tail.nullify();
        _scope = _scope->package_space()->get_blank();
        _name.nullify();
        
        return *this;
    }
    component& id(std::string name, package::category cat = package::BOUND) {
        return id(name, global_scope()->get_by_category(cat));
    }
    component& id(std::string name, package * pkg) {
        _head.nullify();
        _tail.nullify();
        _scope = pkg;
        _name = name;
        assert(is_id());
        return *this;
    }
    //TODO add function to normalize the the global_package associated with each scope
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
        _scope = _scope->package_space()->get_lambda();

        lambda_arg(std::move(new_arg));
        lambda_out(std::move(new_out));
        _name.nullify();

        lambda_arg().parent(this);
        lambda_out().parent(this);

        normalize_child_scope();
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

        _scope = _scope->package_space()->get_expr();
        
        expr_head(std::move(new_head));
        expr_tail(std::move(new_tail));
        _name.nullify();

        expr_head().parent(this);
        expr_tail().parent(this);

        normalize_child_scope();

        assert(is_deep_alloc());
        return *this;
    }

    bool is_id() const { return _scope->is_id(); }
    bool is_expr() const { return _scope->is_expr(); }
    bool is_lambda() const { return _scope->is_lambda(); }
    bool is_init() const { return _scope->is_init(); }
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
    bool is_global() const { return _scope->is_global(); }
    bool is_bound() const { return _scope->is_bound(); }

    component& append(const component& to_add) {
        component copy = to_add;
        return append(std::move(copy));
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
    void collapse() {
        if(is_expr()) {
            if(!expr_tail().is_init()) {
                copy_preserve_parent(std::move(expr_head()));
            } else {
                expr_head().collapse();
                expr_tail().collapse();
            }
        } else if (is_lambda()) {
            lambda_out().collapse();
        }
    }

    std::vector<component*> find_steps();
    
    int evaluate_step();
    int evaluate(int timeout);
    int evaluate_expression();
    int simplify_step();
    int simplify(int timeout);

    int replace_ids(std::string replace_name, const component& replace) {
        return replace_ids(replace_name, replace, _scope->package_space()->get_bound());
    }
    int replace_ids(std::string replace_name, const component& replace, package* replace_scope);

    std::string first_name_not_in_ancestors(std::string base_name) const {
        if(!bound_in_ancestor(base_name))
            return base_name;
        for(unsigned i = 0; i < std::numeric_limits<unsigned>::max(); ++i) {
            std::string name = alt_name(base_name, i);
            if(!bound_in_ancestor(name))
                return name;            
        }
        //memory should be a problem looong before reaching this point
        throw std::runtime_error("number of name variations exceeds integer size");
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
    bool bound_from_above(const std::string& check) const {
        if(is_lambda()) {
            if(lambda_arg().id_name() == check) {
                return false;
            } else {
                return lambda_out().bound_from_above(check);
            }
        } else if (is_expr()) {
            return expr_head().bound_from_above(check)
                || expr_tail().bound_from_above(check);
        } else if (is_id()) {
            if(id_name() == check && is_bound()) {
                return true;
            } else {
                return false;
            }
        } else {
            throw std::logic_error("component not a lambda, expression, or identifier");
        }
    }
    bool bound_above_below(const std::string& check) const {
        if(is_lambda()) {
            if(lambda_arg().id_name() == check) {
                return true;
            } else {
                return lambda_out().bound_above_below(check);
            }
        } else if (is_expr()) {
            return expr_head().bound_above_below(check)
                || expr_tail().bound_above_below(check);
        } else if (is_id()) {
            if(id_name() == check && is_bound()) {
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
    bool lambda_has_arg(std::string check) const {
        if(!is_lambda())
            return false;
        if(lambda_arg().id_name() == check)
            return true;
        return lambda_out().lambda_has_arg(check);
    }
    bool has_unknown() const;
    bool has_unknown(std::vector<std::string>& known) const;
    bool lambda_unknown_before_arg() const;    
    int lambda_unknown_before_arg(const std::string& argname) const;

    std::string to_string() const;
    std::pair<std::string, std::vector<step_string_info>> step_string(const std::vector<component*>& steps) const;
    std::vector<step_string_info> step_string(std::string& out, std::vector<component*>& steps) const;


    bool match_bound(std::string myid, const component& comp, std::string compid) const;
    bool lambda_arg_match(const component& comp) const;
    bool equivalent(const component& comp) const;
    bool operator==(const component& comp) { return equivalent(comp); }
    bool operator!=(const component& comp) { return !equivalent(comp); }

    package::category get_category() const {
        return scope()->get_category();
    }
    void set_category(package::category c) {
        scope(scope()->package_space()->get_by_category(c));
    }
};



}


#endif //LAMBDA_COMPONENT_H