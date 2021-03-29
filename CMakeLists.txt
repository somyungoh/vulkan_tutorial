cmake_minimum_required(VERSION 3.0.0)
project(Vulkan_Tutorial VERSION 0.1.0)


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

# --- include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../VulkanSDK/macOS/include)    # TODO: set this as a user input
include_directories(${GLFW_INCLUDE_DIRS})

# --- library directories
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../VulkanSDK/macOS/lib)    # TODO: set this as a user input

# --- sources
add_executable(Vulkan_Tutorial src/main.cpp)

# --- linking
target_link_libraries(Vulkan_Tutorial Vulkan)
target_link_libraries(Vulkan_Tutorial ${GLFW_LIBRARIES})

# --- other properties
set_property(TARGET Vulkan_Tutorial PROPERTY CXX_STANDARD 11)


# add_subdirectory ("src")

include(CTest)
enable_testing()



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
