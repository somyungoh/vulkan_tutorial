cmake_minimum_required(VERSION 3.0.0)
project(Hello_Triangle VERSION 0.1.0)


# --- Linking
# find through packages
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

# link directories
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../VulkanSDK/macOS/lib)    # TODO: set this as a user input


# --- Includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../VulkanSDK/macOS/include)    # TODO: set this as a user input
include_directories(${GLFW_INCLUDE_DIRS})


# --- Files
set(SOURCES **.h)
set(SOURCES **.cpp)


# --- Output dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# --- Target Properties
# sources
add_executable(Hello_Triangle src/main.cpp src/HelloTriangleApp.cpp)

# linking
target_link_libraries(Hello_Triangle Vulkan)
target_link_libraries(Hello_Triangle ${GLFW_LIBRARIES})

# include dirs
target_include_directories(Hello_Triangle PRIVATE ${CMAKE_SOURCE_DIR}/include)


# --- other properties
set_property(TARGET Hello_Triangle PROPERTY CXX_STANDARD 11)

include(CTest)
enable_testing()


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# export build/complie_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)