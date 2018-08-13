#ifndef LAMBDA_PACKAGE_H
#define LAMBDA_PACKAGE_H

#include <map>
#include <vector>
#include <string>

namespace lambda {

class package;
class global_package;
class component;

/**
 * Stores values of idenitifiers in `packages`. Identifiers that belong to a specific package
 * should store a pointer to that package. There are no nested packages.
 * 
 * Special packages are dummies that indicate a more general attribute (namely global, bound, lambda, expr)
 **/
class package {
public:
    enum category {
        BOUND,
        GLOBAL,
        LAMBDA,
        EXPR,
        PACKAGE,
        EMPTY
    };
    package(global_package* space, std::string name, category c = PACKAGE) {
        pkg_space = space;
        pkg_name = name;
        cat = c;
    }
    package() = delete;
    const component * get_value(std::string key) const;
    void add_value(std::string key, const component& to_add);
    bool is_bound() { return cat == BOUND; }
    bool is_global() { return cat == GLOBAL; }
    bool is_lambda() { return cat == LAMBDA; }
    bool is_expr() { return cat == EXPR; }
    bool is_package() { return cat == PACKAGE; }
    bool is_id() { return cat == PACKAGE || cat == BOUND || cat == GLOBAL; }
    category get_category() { return cat; }
    bool is_init() { return cat != EMPTY; }
    const std::string& name() { return pkg_name; }
    global_package* package_space() { return pkg_space; }
    package* equivalent_in(global_package* other_space);
    static std::string category_string(category cat);
private:
    category cat;
    std::string pkg_name;
    global_package* pkg_space;
    std::map<std::string, component> values;
};

/**
 * stores all identifiers and their values in a program
 **/
class global_package {
private:
    friend lambda::package;
    package base;
    package bound,global,lambda,expr,blank;
    std::map<std::string, package> packages;
public:
    global_package() : 
        base(this, "global", package::PACKAGE), 
        bound(this, "", package::BOUND),
        global(this, "", package::GLOBAL),
        lambda(this, "", package::LAMBDA),
        blank(this, "", package::EMPTY),
        expr(this, "", package::EXPR) {}        

    package * add_package(std::string key);
    package * get_package(std::string key);
    const package * get_package(std::string key) const;

    const component * get_value(std::string key) const;
    const component * get_value(std::string component_key, std::vector<std::string> package_keys) const;
    void add_value(std::string key, const component& to_add);

    package * get_lambda() { return &lambda; }
    package * get_expr() { return &expr; }
    package * get_bound() { return &bound; }
    package * get_global() { return &global; }
    package * get_blank() { return &blank; }
    package * get_by_category(package::category cat);
};

/**singleton used by default in components**/
inline global_package default_space;

}



#endif //LAMBDA_PACKAGE_H