#include "config.hpp"
#include "runner.hpp"
#include <iostream>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "usage: mini-actions <config.pipe>\n";
        return 2;
    }
    try {
        Pipeline p = parse_config(argv[1]);
        return run_pipeline(p);
    } catch (const std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
}
