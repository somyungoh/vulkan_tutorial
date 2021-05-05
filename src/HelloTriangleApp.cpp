#include "HelloTriangleApp.h"
#include "VulkanManager.h"

#include <iostream>
#include <cstring>
#include <stdexcept>

// constructor, destructors
HelloTriangleApp::HelloTriangleApp()
    : WIDTH(800), HEIGHT(600) {};

HelloTriangleApp::~HelloTriangleApp() {};


void HelloTriangleApp::run()
{
    initGLFW();
    initVulkanManager();
    mainLoop();
    cleanup();
}


//::::::::::::::::::::::
//      GLFW            
//::::::::::::::::::::::

// initialize GLFW
void HelloTriangleApp::initGLFW()
{
    if (!glfwInit())
    {
        std::cout << "failed to initialze GLFW" << std::endl;
        return;
    }

    // GLFW is initially deigned for OpenGL, so we need to explicitly tell
    // that we will not create OpenGL context.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // create window
    m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Salut Vulkan Triangle!", nullptr, nullptr);

    if (!m_Window)
    {
        std::cout << "Failed to initialize GLFW window" << std::endl;
        return;
    }
    else
        std::cout << "Successfully initialized GLFW window" << std::endl;
}

void HelloTriangleApp::initVulkanManager()
{
    m_VulkanManager->initVulkan();
}

// main event loop for GLFW
void HelloTriangleApp::mainLoop()
{
    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();
    }
}

void HelloTriangleApp::cleanup()
{
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}