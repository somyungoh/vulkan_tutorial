#include "HelloTriangleApp.h"

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


// -------------------<<  vulkan instance  >>------------------------
//
//  connection between the program and the vulkan library.
//  this will involve specifying hardware specs.
//
// ------------------------------------------------------------------
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
    // extensions
    uint32_t        glfwExtensionCount      = -1;
    const char**    glfwExtensions          = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    vkCreateInfo.ppEnabledExtensionNames    = glfwExtensions;
    vkCreateInfo.enabledExtensionCount      = glfwExtensionCount;
    // validation layers
    if (enableValidationLayers && !checkValidationLayerSupport())
        throw std::runtime_error("Invalid validation layer requested");
    if (enableValidationLayers)
    {
        vkCreateInfo.enabledLayerCount      = static_cast<uint32_t>(validationLayers.size());
        vkCreateInfo.ppEnabledLayerNames    = validationLayers.data();
    }
    else
        vkCreateInfo.enabledLayerCount      = 0;


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


// ------------------------<< Validation Layers >>--------------------------
//
//  This is to handle the possible errors explicitly to avoid crashes.
//  By default, Vulkan has very limited tools for error handling, so
//  it is important to use this well. Possible handlings you can do:
//    o Parameter values 
//    o Track object creation & destruction
//    o Check thread safety
//    o Log function calls and parameters
//    o Tracing Vulkan calls for profiling & replaying
//
//  In practice, you can use this for Debug mode, and disable it in the
//  release version. 
//
// -------------------------------------------------------------------------
bool HelloTriangleApp::checkValidationLayerSupport()
{
    // load all available validation layers
    uint32_t n_LayerCount;
    vkEnumerateInstanceLayerProperties(&n_LayerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(n_LayerCount);
    vkEnumerateInstanceLayerProperties(&n_LayerCount, availableLayers.data());

    // check if our validation layers are in the available list
    for (const char* layer : validationLayers)
    {
        bool isLayerFound = false;
        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layer, layerProperties.layerName) == 0)
            {
                isLayerFound = true;
                std::cout << "Requested Validation Layer found: " << layer << std::endl;
                break;
            }
        }
        if (!isLayerFound)
            return false;
    }

    return true;
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