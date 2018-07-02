#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include "nullable.h"

namespace lambda {

class package;
class component;

class package {
    std::string name;
    std::map<std::string, component> values;
public:
    const component * get_value(std::string);
    component remove_value(std::string);
    component add_value(std::string,const component&);
};

namespace space {
    package bound, global, lambda, expr;
}

class component {
private:
    component* _parent;
    package* _space;
    
    nullable<std::string> _name;

    nullable<component> _head;
    nullable<component> _tail;

public:

    component& copy_preserve_parent(const component& copy) {
        component * temp_parent = parent();
        *this = copy;
        parent() = temp_parent;
    }

    component*& parent() { return _parent; }
    package*& space() { return _space; }
    const component * const parent() const { return _parent; }
    const package * const space() const { return _space; }

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
    component& expr_head(component& copy) {
        assert(is_expr());
        _head.set(copy);
        _head.get().parent() = this;
        return _head.get();
    }
    component& expr_tail(component& copy) {
        assert(is_expr());
        _tail.set(copy);
        _tail.get().parent() = this;
        return _tail.get();
    }

    std::string& id_name() {
        assert(is_id());
        return _name;
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
    component& lambda_arg(component& copy) {
        assert(is_lambda());
        _head.set(copy);
        _head.get().parent() = this;
        return _head.get();
    }
    component& lambda_out(component& copy) {
        assert(is_lambda());
        _tail.set(copy);
        _tail.get().parent() = this;
        return _tail.get();
    }

    component& id(const std::string& name, package * space);
    component& lambda(const component& arg, const component& out);
    component& expr(const component& head, const component& tail);

    bool is_id() const { return _space != &space::expr && _space != &space::lambda; }
    bool is_expr() const { return _space == &space::expr; }
    bool is_lambda() const { return _space == &space::lambda; }
    
    int evaluate_step();
    int evaluate();
    int evaluate_expression();
    int replace_ids(std::string replace_name, const component& replace, package * replace_space = &space::bound);


    std::string to_string();
};













}
















