#pragma once

// common header that defines various configuration and macros

#include <vector>
#include <iostream>

#if defined(NDEBUG)
#   define VERBOSE_LEVEL 0
#elif defined(VERBOSE_LEVEL_MAX)    // level: annoying
#   define VERBOSE_LEVEL 2
#else
#   define VERBOSE_LEVEL 1
#endif

#define PRINT(_str)             std::cout << _str
#define PRINTLN(_str)           std::cout << _str << std::endl
#define PRINT_VERBOSE(_str)     if(VERBOSE_LEVEL > 0) PRINT(_str)
#define PRINTLN_VERBOSE(_str)   if(VERBOSE_LEVEL > 0) PRINTLN(_str)
#define PRINT_BAR_LINE()        PRINTLN("-------------------------------------------------------------------")
#define PRINT_BAR_DOTS()        PRINTLN("...................................................................")
