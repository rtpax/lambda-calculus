
#include "component.h"
#include "package.h"


namespace lambda {

const component * package::get_value(std::string key) const {
    try { //holding out for c++20 map::contains
        return &values.at(key);
    } catch(std::out_of_range) {
        return nullptr;
    }
}

void package::add_value(std::string key, const component& to_add) {
    values[key] = to_add;
}

std::string package::category_string(package::category cat) {
    #define CASE_RETURN(c) case package::c: return #c
    switch(cat) {
        CASE_RETURN(BOUND);
        CASE_RETURN(GLOBAL);
        CASE_RETURN(LAMBDA);
        CASE_RETURN(EXPR);
        CASE_RETURN(PACKAGE);
        CASE_RETURN(EMPTY);
        default: return "UNKNOWN TYPE";
    }
    #undef CASE_RETURN
}

package * package::equivalent_in(global_package * other) {
    switch(cat) {
    case BOUND:
        return &other->bound;
    case GLOBAL:
        return &other->global;
    case LAMBDA:
        return &other->lambda;
    case EXPR:
        return &other->expr;
    case PACKAGE:
        return other->add_package(name());
    case EMPTY:
        return &other->blank;
    default:
        throw std::logic_error("invalid package category");
    }
}





const component * global_package::get_value(std::string key) const {
    const component * ret = base.get_value(key);
    if (ret == nullptr) {
        for(const std::pair<const std::string,package>& p : packages) {
            ret = p.second.get_value(key);
            if(ret != nullptr) {
                return ret;
            }
        }
        return nullptr;
    } else {
        return ret;
    }
}

const component * global_package::get_value(std::string component_key, std::vector<std::string> package_keys) const {
    const component * ret;
    for(std::vector<std::string>::const_reverse_iterator pki = package_keys.crbegin(); pki != package_keys.crend(); ++pki) {
        const package * p = get_package(*pki);
        if(p == nullptr) {
            throw std::logic_error("attempted to access nonexistant package");
        }
        ret = p->get_value(component_key);
        if(ret != nullptr)
            return ret;
    }

    ret = base.get_value(component_key);
    if (ret == nullptr) {
        for(const std::pair<const std::string,package>& p : packages) {
            ret = p.second.get_value(component_key);
            if(ret != nullptr) {
                return ret;
            }
        }
        return nullptr;
    } else {
        return ret;
    }
}

void global_package::add_value(std::string key, const component& to_add) {        
    base.add_value(key, to_add);
}

package * global_package::add_package(std::string key) {
    if(key == "global")
        return &base;
    if(packages.find(key) == packages.end()) {
        packages.insert(make_pair(key, package(this, key)));
    }
    return &packages.at(key);
}

const package * global_package::get_package(std::string key) const {
    if(key == "global")
        return &base;
    try {
        return &packages.at(key);
    } catch (std::out_of_range) {
        return nullptr;
    }
}

package * global_package::get_package(std::string key) {
    if(key == "global")
        return &base;
    try {
        return &packages.at(key);
    } catch (std::out_of_range) {
        return nullptr;
    }
}

package * global_package::get_by_category(package::category cat) {
    switch (cat) {
    case package::BOUND:
        return &bound;
    case package::EXPR:
        return &expr;
    case package::LAMBDA:
        return &lambda;
    case package::GLOBAL:
        return &global;
    case package::PACKAGE:
        throw std::logic_error("cannot use get_by_category with type PACKAGE");
    case package::EMPTY:
        return &blank;
    default:
        throw std::logic_error("cannot use get_by_category with package of indeterminate_type");
    }
}

}
