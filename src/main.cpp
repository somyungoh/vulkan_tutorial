#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

int main () {

    glfwInit();

    GLFWwindow* window = glfwCreateWindow(400, 400, "Hello Vulkan", nullptr, nullptr);

    uint32_t extentionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extentionCount, nullptr);

    std::cout << extentionCount << " extentions supported.\n";

    glm::mat4 matrix;
    glm::vec4 vector;
    auto test = matrix * vector;

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}