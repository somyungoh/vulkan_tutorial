#include "HelloTriangleApp.h"
#include "VulkanManager.h"
#include "Common.h"

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

static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<VulkanManager*>(glfwGetWindowUserPointer(window));
    app->setFrameBufferResized(true);
}

//::::::::::::::::::::::
//      GLFW            
//::::::::::::::::::::::

// initialize GLFW
void HelloTriangleApp::initGLFW()
{
    PRINT_BAR_LINE();

    if (!glfwInit())
    {
        PRINTLN("failed to initialze GLFW");
        return;
    }

    // GLFW is initially deigned for OpenGL, so we need to explicitly tell
    // that we will not create OpenGL context.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // create window
    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Salut Vulkan Triangle!", nullptr, nullptr);

    // explicit window size handling for Vulkan
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

    if (!m_window)
    {
        PRINTLN("Failed to initialize GLFW window");
        return;
    }
    else
        PRINTLN("Successfully initialized GLFW window");
}

void HelloTriangleApp::initVulkanManager()
{
    m_VulkanManager = new VulkanManager();
    m_VulkanManager->initVulkan(m_window);
}

// main event loop for GLFW
void HelloTriangleApp::mainLoop()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        m_VulkanManager->drawFrame();
    }
}

void HelloTriangleApp::cleanVulkanManager()
{
    m_VulkanManager->cleanVulkan();
    delete m_VulkanManager;
}

void HelloTriangleApp::cleanup()
{
    cleanVulkanManager();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}