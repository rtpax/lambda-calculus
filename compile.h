#ifndef LAMBDA_COMPILE_H
#define LAMBDA_COMPILE_H

#include "token.h"
#include "component.h"

namespace lambda {

global_package * load_file(std::string filename) {
    
}

int evaluate_line(std::vector<token> line, int max_steps = 1024);


}


#endif //LAMBDA_COMPILE_H