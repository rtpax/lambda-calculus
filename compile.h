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
    identifier base; //if base.id_f != none, sub is empty
    std::vector<expression> sub;
};

struct lambda {
    std::vector<identifier> args;
    expression output;
};

struct package {
    std::vector<identifier> contents;
};

