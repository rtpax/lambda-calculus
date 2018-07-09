#include <string>
#include <iostream>

inline void emit_error(std::string message, int line, std::string file) {
    std::cerr << "error at line " << line << " in file '" << file << "': " << message << "\n";
}

inline void emit_warning(std::string message, int line, std::string file) {
    std::cerr << "warning at line " << line << " in file '" << file << "': " << message << "\n";
}