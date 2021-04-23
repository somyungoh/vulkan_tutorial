#include "HelloTriangleApp.h"

#include <iostream>
#include <vector>

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
    uint32_t        glfwExtensionCount      = -1;
    const char**    glfwExtensions          = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    vkCreateInfo.ppEnabledExtensionNames    = glfwExtensions;
    vkCreateInfo.enabledExtensionCount      = glfwExtensionCount;
    vkCreateInfo.enabledLayerCount          = 0;    // TODO: what's this?


#if 1
    // optional) available extension check
    uint32_t n_ExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &n_ExtensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(n_ExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &n_ExtensionCount, extensions.data());

    // print vulkan ext.
    std::cout << "Available Vulkan Extensions:" << std::endl;
    for (const auto& extension : extensions)
        std::cout << "\t" << extension.extensionName << std::endl;
    // print glfw ext.
    std::cout << "Required GLFW Extensions:" << std::endl;
    for (int i = 0; i < glfwExtensionCount; i++)
        std::cout << "\t" << glfwExtensions[i] << std::endl;
#endif

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
    // Vulkan
    vkDestroyInstance(m_VkInstance, nullptr);

    // GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}