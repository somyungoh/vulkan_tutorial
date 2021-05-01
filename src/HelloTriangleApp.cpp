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
    createDebugMessenger();
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
    std::vector<const char*> extensions;
    getRequiredVKExtensions(extensions);
    vkCreateInfo.ppEnabledExtensionNames    = extensions.data();
    vkCreateInfo.enabledExtensionCount      = static_cast<uint32_t>(extensions.size());
    // validation layers
    if (enableValidationLayers && !checkValidationLayerSupport())
        throw std::runtime_error("Invalid validation layer requested");
    if (enableValidationLayers)
    {
        vkCreateInfo.enabledLayerCount      = static_cast<uint32_t>(m_validationLayers.size());
        vkCreateInfo.ppEnabledLayerNames    = m_validationLayers.data();
    }
    else
        vkCreateInfo.enabledLayerCount      = 0;

#if 1
    // optional) available extension check
    uint32_t n_ExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &n_ExtensionCount, nullptr);
    std::vector<VkExtensionProperties> vk_extensions(n_ExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &n_ExtensionCount, vk_extensions.data());

    // print vulkan ext.
    std::cout << "Available Vulkan Extensions:" << std::endl;
    for (const auto& extension : vk_extensions)
        std::cout << "\t" << extension.extensionName << std::endl;
    // print glfw ext.
    //std::cout << "Required GLFW Extensions:" << std::endl;
    //for (int i = 0; i < glfwExtensionCount; i++)
    //   std::cout << "\t" << glfwExtensions[i] << std::endl;
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
    for (const char* layer : m_validationLayers)
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

void    HelloTriangleApp::getRequiredVKExtensions(std::vector<const char*>& ext)
{
    uint32_t        glfwExtensionCount = 0;
    const char**    glfwExtensions;
    glfwExtensions  = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // add glfw extensions
    // NOTE: is this safe?
    ext = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // add debug messenger extension (allows message callbacks for validation layers)
    if (enableValidationLayers)
        ext.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}


// ----------------------------<< Debug Messenger >>---------------------------
//
//  Used along with the validation layers, this extension will allow for
//  validation layers to print back various messages. It holds various
//  messsage type/severity that you wish to recieve, and a callback function.
//
// ----------------------------------------------------------------------------
// Look at https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VK_EXT_debug_utils
// for much more different ways to setup debug messenger.
void HelloTriangleApp::createDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo {};
    createInfo.sType            = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity  = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    createInfo.messageType      = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback  = debugCallback;
    createInfo.pUserData        = nullptr;

    // create messenger via vkCreateDebugUtilsMessengerEXT
    if (createDebugUtilsMessengerEXT(m_VkInstance, &createInfo, nullptr, &m_debugMessenger)
        != VK_SUCCESS)
        throw std::runtime_error("Failed set up debug messenger!");
    else
        std::cout << "Successfully created Debug Messenger." << std::endl;
}

// As <vkCreateDebugUtilsMessengerEXT> is a extension, it's not automatically
// loaded, so we need to explicitly look up and get the function address... sigh
VkResult HelloTriangleApp::createDebugUtilsMessengerEXT(
    VkInstance                                  instance,
    const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugUtilsMessengerEXT*                   pDebugMessenger)
{
    auto pFunc  = (PFN_vkCreateDebugUtilsMessengerEXT)
                   vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (pFunc != nullptr)
        return pFunc(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

VkResult     HelloTriangleApp::destroyDebugUtilsMessengerEXT(
    VkInstance                      instance,
    const VkDebugUtilsMessengerEXT* pDebugMessenger,
    const VkAllocationCallbacks*    pAllocator)
{
    auto pFunc  = (PFN_vkDestroyDebugUtilsMessengerEXT)
                   vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (pFunc != nullptr)
    {
        pFunc(instance, *pDebugMessenger, pAllocator);
        return VK_SUCCESS;
    }
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

// Message callback funtion that can be tied to the validation layer.
// The callback is registered in member 'm_debugMessenger'
VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApp::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT          msgSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                 msgType,
    const VkDebugUtilsMessengerCallbackDataEXT*     pCallbackData,
    void*                                           pUserData)
{
    // Message severity level ( msgSeverity )
    std::cerr << "Validation Layer|";
    if (msgSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        std::cerr << "ERROR|";
    else if (msgSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        std::cerr << "WARNING|";
    else if (msgSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        std::cerr << "INFO|";
    else if (msgSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        std::cerr << "VERBOSE|";

    // Message type ( msgType )
    if (msgType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)  // spec. voilation / mistakes
        std::cerr << "VALIDATION|";
    else if (msgType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)  // non-optimal use of vulkan
        std::cerr << "PERFORMANCE|";
    else if (msgType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)    // non above
        std::cerr << "GENERAL|";

    // Message itself (pCallbackData->pMessage)
    std::cout << pCallbackData->pMessage << std::endl;

    // Your own data (pUserData)
    (void)pUserData;

    // return: should validation layer message be aborted?
    return VK_FALSE;
}


// application exit
void    HelloTriangleApp::cleanup()
{
    // Vulkan
    // extensions must be destroyed before vulkan instance
    if (enableValidationLayers)
        destroyDebugUtilsMessengerEXT(m_VkInstance, &m_debugMessenger, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);

    // GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Successfully cleaning up the program..." << std::endl;
}