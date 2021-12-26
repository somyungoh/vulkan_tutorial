#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

// forward declaration
class VulkanManager;

class MyApp
{
public:
    MyApp();
    virtual ~MyApp();

    void run();

private:
    void    initGLFW();
    void    initVulkanManager();
    void    mainLoop();
    void    cleanVulkanManager();
    void    cleanup();

    // GLFW Window
    GLFWwindow*     m_window;
    const uint32_t  WIDTH;
    const uint32_t  HEIGHT;

    // Vulkan Manager
    VulkanManager*  m_VulkanManager;
};