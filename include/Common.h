#pragma once

// common header that defines various configuration and macros

#include <vector>
#include <iostream>

#if defined(NDEBUG)
#   define VERBOSE 0
#else
#   define VERBOSE 1
#endif

#define PRINT(_str)             std::cout << _str
#define PRINTLN(_str)           std::cout << _str << std::endl
#define PRINT_VERBOSE(_str)     if(VERBOSE) PRINT(_str)
#define PRINTLN_VERBOSE(_str)   if(VERBOSE) PRINTLN(_str)
#define PRINT_BAR_LINE()        PRINTLN("-------------------------------------------------------------------")
#define PRINT_BAR_DOTS()        PRINTLN("...................................................................")
