
#include "compile.h"

using namespace lambda;

int main(int argc, char ** argv) {
    if(argc <= 1) {
        std::cout << "must input a file";
        return 1;
    }
    load_file(argv[1]);
    return 0;
}

