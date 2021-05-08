#include "VulkanManager.h"
#include "Common.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <map>

VulkanManager::VulkanManager() :
    m_device(VK_NULL_HANDLE),
    m_validationLayers({ "VK_LAYER_KHRONOS_validation" })
    {};


// -------------------<<  Bonjour Vulkan!  >>------------------------
//
//  This implementation is following / modified version of the 
//  "Vulkan Tutorial" https://vulkan-tutorial.com
//
// ------------------------------------------------------------------

void VulkanManager::initVulkan()
{
    createVulkanInstance();
    createDebugMessenger();
    loadPhysicalDevice();
}


// -------------------<<  Vulkan Instance  >>------------------------
//
//  connection between the program and the vulkan library.
//  this will involve specifying hardware specs.
//
// ------------------------------------------------------------------

void VulkanManager::createVulkanInstance()
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
    std::vector<const char*> VkExtensions   = loadVKExtensions();
    vkCreateInfo.ppEnabledExtensionNames    = VkExtensions.data();
    vkCreateInfo.enabledExtensionCount      = static_cast<uint32_t>(VkExtensions.size());
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
    PRINT_BAR();
#if 1
    // optional) available extension check
    uint32_t n_ExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &n_ExtensionCount, nullptr);
    std::vector<VkExtensionProperties> vk_extensions(n_ExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &n_ExtensionCount, vk_extensions.data());

    // print vulkan ext.
    PRINTLN_VERBOSE("Available Vulkan Extensions:");
    for (const auto& extension : vk_extensions)
        PRINTLN("\t" << extension.extensionName);
    // print glfw ext.
#endif

    // this is a general pattern of creating a vulkan object:
    VkResult result = vkCreateInstance(
        &vkCreateInfo,  // pointer to a struct that defines the object
        nullptr,        // pointer to custom allocator callbacks
        &m_VkInstance   // pointer to a variable that will store the object
    );

    if (result != VK_SUCCESS)
    {
        PRINTLN("Failed to create Vulkan Instance");
        return;
    }

    PRINTLN_VERBOSE("Successfully created Vulkan Instance");
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

bool VulkanManager::checkValidationLayerSupport()
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

std::vector<const char*> VulkanManager::loadVKExtensions()
{
    // pass required glfw extension
    uint32_t        glfwExtensionCount = 0;
    const char**    glfwExtensions;
    glfwExtensions  = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // init VkExtensions by pointing to glfwExtensions
    std::vector<const char*> VkExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // add debug messenger extension (allows message callbacks for validation layers)
    if (enableValidationLayers)
        VkExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return VkExtensions;
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
void VulkanManager::createDebugMessenger()
{
    if (!enableValidationLayers) return;

    PRINT_BAR();

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
VkResult VulkanManager::createDebugUtilsMessengerEXT(
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

VkResult     VulkanManager::destroyDebugUtilsMessengerEXT(
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
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanManager::debugCallback(
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
    PRINTLN(pCallbackData->pMessage);

    // Your own data (pUserData)
    (void)pUserData;

    // return: should validation layer message be aborted?
    return VK_FALSE;
}


// -------------------<<  Physical Device  >>------------------------
//
//  Search for available GPU device and select one. It is loaded
//  in VkPhysicalDevice.
//
// ------------------------------------------------------------------

void VulkanManager::loadPhysicalDevice()
{
    PRINT_BAR();

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());

    if (deviceCount == 0)
        throw std::runtime_error("Failed to find GPUs with Vulkan Support!");

    // an ordered map, allows multiple elements to have same keys
    std::multimap<int, VkPhysicalDevice> candidates;

    // score each available devices to pick a best one
    PRINTLN_VERBOSE("Starting to score available GPUs...");

    for (const auto& device : devices)
    {
        uint32_t score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    // if (candidates.size() == 1){
    //     m_device = candidates.begin()->second;
    // }
    if (candidates.rbegin()->first > 0)
        m_device = candidates.rbegin()->second;
    else if (m_device == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to find a suitable GPU!");
}

u_int32_t VulkanManager::rateDeviceSuitability(VkPhysicalDevice device)
{
    PRINT_BAR();

    // Here, these queries gives much more information than just the
    // GPU information (texture compression, multi-viewport rendering)
    VkPhysicalDeviceProperties  deviceProperties;
    VkPhysicalDeviceFeatures    deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // GPU Details
    PRINTLN_VERBOSE("GPU Name: " << deviceProperties.deviceName);
    PRINTLN_VERBOSE("Scoring:");

    u_int32_t score = 0;

    // Criteria 1) Obviously, discrete GPUs are always better
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 100;   // no idea how the scale should be...
        PRINTLN_VERBOSE("\tDiscrete GPU - score 100");
    }
    else
        PRINTLN_VERBOSE("\tNot a Discrete GPU - score 0");
    // Criteria 2) Maximum texture size
    score += deviceProperties.limits.maxImageDimension2D;
    PRINTLN_VERBOSE("\tMax 2D texture dimension: " << deviceProperties.limits.maxImageDimension2D);

    // Criteria 3) Geometry shaders
    if (!deviceFeatures.geometryShader)
    {
        score -= 100;   // lol.
        PRINTLN_VERBOSE("\tGeometry Shader not available, minus score 100...");
    }

    PRINTLN_VERBOSE("\tThe final score is: " << score);

    return score;
}


// --------------------------<<  Exit  >>---------------------------
//
//  VkPhysicalDevice - automatically handled
//
// ------------------------------------------------------------------

void VulkanManager::cleanVulkan()
{
    // Vulkan
    // extensions must be destroyed before vulkan instance
    if (enableValidationLayers)
        destroyDebugUtilsMessengerEXT(m_VkInstance, &m_debugMessenger, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);
    
    PRINTLN("Successfully cleaning up Vulkan...");
}
