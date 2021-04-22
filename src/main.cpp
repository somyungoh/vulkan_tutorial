#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "HelloTriangleApp.h"

int main()
{
    HelloTriangleApp app;

    try
    {
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}