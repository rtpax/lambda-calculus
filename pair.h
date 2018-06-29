
#include "compile.h"

struct pair_identifier;
struct expression_pair;
struct lambda_pair;
struct pair;

struct pair_identifier {
    std::string name;
    package& scope;
    expression_pair& parent;
};

struct expression_pair {
    nullable<pair> parent;//lambda or expression
    nullable<pair> value;//expression or identifier
    nullable<expression_pair> next;
};

struct lambda_pair {
    pair_identifier arg;
    nullable<pair> output;
};

struct pair {
    enum {
        LAMBDA,
        EXPRESSION,
        IDENTIFIER,
        NONE
    } type;
    union {
        pair_identifier i;
        lambda_pair l;
        expression_pair e;
    } value;
};


pair normal_form(expression);
pair normal_form(lambda);