#include "token.h"
#include <optional>
#include <vector>
#include <string>
#include "nullable.h"

enum class identifier_flag {
    bound,
    free,
    none
};

struct lambda;

struct identifier {
    identifier_flag id_f;
    std::string name;
    nullable<lambda> value;
};

struct expression {
    identifier base;
    std::vector<expression> sub;
};

struct lambda {
    std::vector<std::string> args;
    expression output;
};

struct package {
    std::vector<identifier> contents;
};

struct global {
    std::vector<package> packages;
    std::vector<identifier> contents;
}

struct expression_pair;
struct expression_pair {
    identifier base;
    nullable<expression_pair> sub; 
}

struct lambda_pair {
    identifier arg;
    expression_pair output;
}


expression_pair normal_form(expression);
lambda_pair normal_form(lambda);

