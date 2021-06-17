#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

// forward declaration
class VulkanManager;

class HelloTriangleApp
{
public:
    HelloTriangleApp();
    virtual ~HelloTriangleApp();

    void run();

private:
    void    initGLFW();
    void    initVulkanManager();
    void    mainLoop();
    void    cleanVulkanManager();
    void    cleanup();

    // GLFW Window
    GLFWwindow*     m_Window;
    const uint32_t  WIDTH;
    const uint32_t  HEIGHT;

    // Vulkan Manager
    VulkanManager*  m_VulkanManager;
};