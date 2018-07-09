#ifndef LAMBDA_COMPILE_H
#define LAMBDA_COMPILE_H

#include "token.h"
#include "component.h"

namespace lambda {

void load_file(std::string filename);

int evaluate_line(std::vector<token> line, int max_steps = 1024);


}


#endif