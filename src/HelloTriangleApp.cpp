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
    window = glfwCreateWindow(WIDTH, HEIGHT, "Salut Vulkan Triangle!", nullptr, nullptr);

    if (!window)
    {
        std::cout << "Failed to initialize GLFW window" << std::endl;
        return;
    }
    else
        std::cout << "Successfully initialized GLFW window" << std::endl;
}

// main event loop for GLFW
void HelloTriangleApp::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

//::::::::::::::::::::::
//      Vulkan          
//::::::::::::::::::::::

void HelloTriangleApp::initVulkan()
{
    createVulkanInstance();

    std::cout << "Successfully initialized Vulkan" << std::endl;
}

// connection between the program and the vulkan library.
// this will involve specifying hardware specs.
void HelloTriangleApp::createVulkanInstance()
{
    // in vulkan, in many cases, data is passed through different structs
    // instead of function parameters. Each vulkan struct requires to specify
    // the type into the member 'sType'.

    // struct: application information
    VkApplicationInfo vkAppInfo{};
    vkAppInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkAppInfo.pApplicationName    = "Hello Triangle";
    vkAppInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
    vkAppInfo.pEngineName         = "No Engine";
    vkAppInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
    vkAppInfo.apiVersion          = VK_API_VERSION_1_0;

    // struct: vulkan instance information
    VkInstanceCreateInfo vkCreateInfo{};
    vkCreateInfo.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkCreateInfo.pApplicationInfo           = &vkAppInfo;
    uint32_t glfwExtensionCount;
    vkCreateInfo.ppEnabledExtensionNames    = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    vkCreateInfo.enabledExtensionCount      = glfwExtensionCount;
    vkCreateInfo.enabledLayerCount          = 0;    // TODO: what's this?

    // vulkan instance.
    // this is a general pattern of creating a vulkan object:
    VkResult result = vkCreateInstance(
        &vkCreateInfo,  // pointer to a struct that defines the object
        nullptr,        // pointer to custom allocator callbacks
        &m_VkInstance   // pointer to a variable that will store the object
    );

    if (result != VK_SUCCESS)
    {
        std::cout << "Failed to create Vulkan Instance" << std::endl;
        return;
    }

    std::cout << "Successfully created Vulkan Instance" << std::endl;
}

// application exit
void HelloTriangleApp::cleanup()
{
    // GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}