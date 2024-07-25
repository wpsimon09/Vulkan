#include <iostream>

#include "HelloTriangle.hpp"

int main() {

    HelloTriangle app;

    try {
        app.run();
    }catch(const std::exception &e) {
        std::cerr<<e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}