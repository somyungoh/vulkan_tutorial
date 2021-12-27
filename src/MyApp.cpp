#include "MyApp.h"
#include "VulkanManager.h"
#include "Common.h"

#include <iostream>
#include <cstring>
#include <stdexcept>
#include <chrono>


//----------------------------------------------------------------------

MyApp::MyApp() : m_width(800), m_height(600)
{
};

MyApp::~MyApp()
{
};

//----------------------------------------------------------------------

void MyApp::run()
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

void MyApp::initGLFW()
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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // create window
    m_window = glfwCreateWindow(m_width, m_height, "Vulkan Window", nullptr, nullptr);

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

void MyApp::initVulkanManager()
{
    m_VulkanManager = new VulkanManager();

    m_VulkanManager->initVulkan(m_window);
}

void MyApp::mainLoop()
{
    // fps timer setup
    uint32_t    frames = 0;
    double      fps = 0;
    auto        prevTime = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        m_VulkanManager->drawFrame();

        // update FPS
        if (frames > 10)
        {
            auto    currentTime = std::chrono::high_resolution_clock::now();
            double  dt = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();

            fps = (double)frames / dt;

            prevTime = currentTime;
            frames = 0;
        }
        frames++;

        // display FPS in the Window title
        char str_buffer[64];
        sprintf(str_buffer, "Bonjour Vulkan!\t fps: %.2f", fps);
        glfwSetWindowTitle(m_window, str_buffer);
    }
}

void MyApp::cleanVulkanManager()
{
    m_VulkanManager->cleanVulkan();

    delete m_VulkanManager;
}

void MyApp::cleanup()
{
    cleanVulkanManager();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}