#include "HelloTriangleApp.h"

#include <iostream>

// constructor, destructors
HelloTriangleApp::HelloTriangleApp()
    : WIDTH(800), HEIGHT(600) {};

HelloTriangleApp::~HelloTriangleApp() {};

void HelloTriangleApp::run()
{
    initGLFW();
    initVulkan();
    mainLoop();
    cleanup();
}

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
    window = glfwCreateWindow(WIDTH, HEIGHT, "Salut Vulkan Triangle!", nullptr, nullptr);

    if (!window)
    {
        std::cout << "Failed to initialize GLFW window" << std::endl;
        return;
    }
    else
        std::cout << "GLFW window succesfully initialized" << std::endl;
}

// main event loop for GLFW
void HelloTriangleApp::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void HelloTriangleApp::initVulkan()
{

}

// application exit
void HelloTriangleApp::cleanup()
{
    // GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}