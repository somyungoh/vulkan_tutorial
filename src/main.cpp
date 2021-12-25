#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "MyApp.h"

int main()
{
    MyApp app;

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