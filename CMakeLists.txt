cmake_minimum_required(VERSION 3.0.0)
project(Hello_Triangle VERSION 0.1.0)


# --- required packages
# vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)

# glfw : UNIX system (linux, mac) 
if (UNIX)
    # this creates CMake commands to find pkg-config packages.
    # required by the system. (ex. brew install pkg-config / sudo apt-get install pkg-config)
    find_package(glfw3 3.3 REQUIRED)
    find_package(glm REQUIRED)
    # pkg_search_module(GLFW REQUIRED glfw3)
endif (UNIX)

# --- include directories (All Targets)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../VulkanSDK/macOS/include)    # TODO: set this as a user input
include_directories(${GLFW_INCLUDE_DIRS})

# --- files
set(SOURCES **.h)
set(SOURCES **.cpp)

# --- library directories
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../VulkanSDK/macOS/lib)    # TODO: set this as a user input


# --- sources
add_executable(Hello_Triangle src/main.cpp)

# --- linking
target_link_libraries(Hello_Triangle Vulkan)
target_link_libraries(Hello_Triangle ${GLFW_LIBRARIES})

# --- include dirs (Target Specific)
target_include_directories(Hello_Triangle PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


# --- other properties
set_property(TARGET Hello_Triangle PROPERTY CXX_STANDARD 11)


# add_subdirectory ("src")

include(CTest)
enable_testing()


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# export build/complie_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)