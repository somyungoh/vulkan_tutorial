#include "VulkanManager.h"
#include "Common.h"

// required for window surface (by Vulkan)
// reference) https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#vkCreateMacOSSurfaceMVK
#if defined(_WIN32) || defined(_WIN64)
#   define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__APPLE__)
#   define VK_USE_PLATFORM_MACOS_MVK
#endif  // defined(_WIN32) || defined(_WIN64)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// required for window surface (by GLFW)
// reference) https://www.glfw.org/docs/3.3/group__native.html
#if defined(_WIN32) || defined(_WIN64)
#   define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
#   define GLFW_EXPOSE_NATIVE_COCOA
#endif  // defined(_WIN32) || defined(_WIN64)
#include <GLFW/glfw3native.h>

// std
#include <optional>
#include <map>
#include <set>


struct QueueFamilyIndices
{
    // std::optional - c++17 extension, which contains nothing until
    // something is assigned, and can be checked using has_value()
    std::optional<u_int32_t> graphicsFamily;
    std::optional<u_int32_t> presentationFamily;

    bool isComplete() { return graphicsFamily.has_value() && presentationFamily.has_value(); }
};

struct SwapchainSupportDetails
{
   VkSurfaceCapabilitiesKHR surfaceCapabilities;
   std::vector<VkSurfaceFormatKHR> formats;
   std::vector<VkPresentModeKHR> presentModes;
};


// -------------------<<  Bonjour Vulkan!  >>------------------------
//
//  This implementation is following / modified version of the 
//  "Vulkan Tutorial" https://vulkan-tutorial.com
//
// ------------------------------------------------------------------

VulkanManager::VulkanManager() :
    m_physicalDevice(VK_NULL_HANDLE),
    m_device(VK_NULL_HANDLE),
    m_validationLayers({ "VK_LAYER_KHRONOS_validation" }),
    m_deviceExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME })
    {
    };


void VulkanManager::initVulkan(GLFWwindow* window)
{
    PRINT_BAR_LINE();
    PRINTLN("Start initializing vulkan manager.");

    bool result = true;

    result &= createVulkanInstance();
    result &= createDebugMessenger();
    result &= createWindowSurface(window);
    result &= loadPhysicalDevice();
    result &= createLogicalDevice();

    PRINT_BAR_DOTS();
    if (result)
        PRINTLN("Vulkan Manager initialization finished successfully");
    else
        PRINTLN("Vulkan Manager initialization finished with errors");
    PRINT_BAR_LINE();
}


// -------------------<<  Vulkan Instance  >>------------------------
//
//  connection between the program and the vulkan library.
//  this will involve specifying hardware specs.
//
// ------------------------------------------------------------------

bool VulkanManager::createVulkanInstance()
{
    // in vulkan, in many cases, data is passed through different structs
    // instead of function parameters. Each vulkan struct requires to specify
    // the type into the member 'sType'.

    PRINT_BAR_DOTS();

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
#if 1
    // optional) available extension check
    uint32_t n_ExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &n_ExtensionCount, nullptr);
    std::vector<VkExtensionProperties> vk_extensions(n_ExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &n_ExtensionCount, vk_extensions.data());

    // print vulkan ext.
    PRINTLN_VERBOSE("Available Vulkan Extensions:");
    for (const auto& extension : vk_extensions)
        PRINTLN_VERBOSE("\t" << extension.extensionName);
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
        return false;
    }

    PRINTLN("Created Vulkan Instance");
    return true;
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
                PRINTLN("Requested Validation Layer found: " << layer);
                break;
            }
        }
        if (!isLayerFound){
            PRINTLN("Requested Validation Layer not found: " << layer);
            return false;
        }
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
bool VulkanManager::createDebugMessenger()
{
    if (!enableValidationLayers)
        return true;

    PRINT_BAR_DOTS();

    VkDebugUtilsMessengerCreateInfoEXT createInfo {};
    createInfo.sType            = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity  = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
#if defined(VERBOSE_LEVEL_MAX)
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
#endif
    createInfo.messageType      = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback  = debugCallback;
    createInfo.pUserData        = nullptr;

    // create messenger via vkCreateDebugUtilsMessengerEXT
    if (createDebugUtilsMessengerEXT(m_VkInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed set up debug messenger!");
        return false;
    }
    else
        PRINTLN_VERBOSE("Successfully created Debug Messenger");

    return true;
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
    PRINT("Validation Layer|");
    if (msgSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        PRINT("ERROR|");
    else if (msgSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        PRINT("WARNING|");
    else if (msgSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        PRINT("INFO|");
#if defined(VERBOSE_LEVEL_MAX)
    else if (msgSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        PRINT("VERBOSE|");
#endif

    // Message type ( msgType )
    if (msgType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)  // spec. voilation / mistakes
        PRINT("VALIDATION|");
    else if (msgType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)  // non-optimal use of vulkan
        PRINT("PERFORMANCE|");
    else if (msgType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)    // non above
        PRINT("GENERAL|");

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

bool VulkanManager::loadPhysicalDevice()
{
    PRINT_BAR_DOTS();

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());

    if (deviceCount == 0)
        throw std::runtime_error("Failed to find GPUs with Vulkan Support!");

    // an ordered map, allows multiple elements to have same keys
    std::multimap<int, VkPhysicalDevice> candidates;

    // score each available devices to pick a best one
    for (const auto& device : devices)
    {
        uint32_t score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    if (candidates.rbegin()->first > 0 && isDeviceSuitable(candidates.rbegin()->second))
        m_physicalDevice = candidates.rbegin()->second;
    else if (m_physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
        return false;
    }

    VkPhysicalDeviceProperties  deviceProperties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);
    PRINTLN("Loaded physical device - " << deviceProperties.deviceName);
    return true;
}

u_int32_t VulkanManager::rateDeviceSuitability(VkPhysicalDevice device)
{
    // Here, these queries gives much more information than just the
    // GPU information (texture compression, multi-viewport rendering)
    VkPhysicalDeviceProperties  deviceProperties;
    VkPhysicalDeviceFeatures    deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // GPU Details
    PRINTLN("GPU Name: " << deviceProperties.deviceName);
    PRINTLN("Scoring:");

    u_int32_t score = 0;

    // Criteria 1) Obviously, discrete GPUs are always better
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 100;   // no idea how the scale should be...
        PRINTLN("\tDiscrete GPU - score 100");
    }
    else
        PRINTLN("\tNot a Discrete GPU - score 0");

    // Criteria 2) Maximum texture size
    score += deviceProperties.limits.maxImageDimension2D;
    PRINTLN("\tMax 2D texture dimension: " << deviceProperties.limits.maxImageDimension2D);

    // Criteria 3) Geometry shaders
    if (!deviceFeatures.geometryShader)
    {
        score -= 100;   // lol.
        PRINTLN("\tGeometry Shader not available, minus score 100...");
    }

    PRINTLN("Final score: " << score);

    return score;
}

bool VulkanManager::isDeviceSuitable(VkPhysicalDevice device)
{
    // queue family
    QueueFamilyIndices indices  = findQueueFamilies(device);

    // extension: swapchain support
    bool extensionSupported     = checkDeviceExtensionSupport(device);
    bool swapchainAdequate      = false;
    if (extensionSupported)
    {
        // swap chain
        SwapchainSupportDetails swapchainDetails = querySwapChainSupport(device);
        swapchainAdequate = !swapchainDetails.formats.empty() &&
                            !swapchainDetails.presentModes.empty();
        if (swapchainAdequate)
            PRINTLN("Extension) Swapchain supported for this device");
    }

    return indices.isComplete() && extensionSupported && swapchainAdequate;
}

// this will check whether the physical device supports everything
// that is required, defined in our vector m_deviceExtensions
bool VulkanManager::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t n_extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &n_extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(n_extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &n_extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

    // ease out if the device supports our required extensions
    for (const auto& extension : availableExtensions)
    {
        if (requiredExtensions.erase(extension.extensionName))
            PRINTLN("Extension) Required extension \"" << extension.extensionName << "\" supported.");
    }
    return requiredExtensions.empty();
}


// --------------------<<  Queue Families  >>----------------------------
//
//  In Vulkan, everything in GPU - from data loading, drawing - require
//  commands to be submitted to a queue. There are different types of
//  queues that allows a subset of commands, and this is called the
//  "Queue Family". For now, we only search for 'Graphics commands'.
//
// ----------------------------------------------------------------------

QueueFamilyIndices VulkanManager::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    u_int32_t n_queueFamily;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &n_queueFamily, nullptr);

    // VkQueueFamilyProperties will contain the details of the queue family
    std::vector<VkQueueFamilyProperties> queueFamilies(n_queueFamily);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &n_queueFamily, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        // not all physical device supports presentation support, so we need to query the availability.
        VkBool32 presentationSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_windowSurface, &presentationSupport);

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;
        if (presentationSupport)
            indices.presentationFamily = i;
        i++;
    }

    if (indices.isComplete())
    {
        PRINTLN("Queue Family) Graphics QF available: index " << indices.graphicsFamily.value());
        PRINTLN("Queue Family) Presentation QF available: index " << indices.presentationFamily.value());
    }

    return indices;
}


// ----------------------<<  Logical Device  >>-----------------------------
//
//  After selecting a physical device, we need to setup a 'logical device'
//  to interface with it. Here, we specify which queues to create from
//  the queue family that we quried previously.
//
// -------------------------------------------------------------------------

bool VulkanManager::createLogicalDevice()
{
    PRINT_BAR_DOTS();

    // 1. specify the queue to be created
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentationFamily.value()
    };

    // Priorities are assigned to queues which will later influence the
    // scheduling of command buffer execution. Pretty nice.
    float queuePriority = 1.0f;     // between (0.0 ~ 1.0)
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType               = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex    = queueFamily;
        queueCreateInfo.queueCount          = 1;    // why is this only 1? It's because
        // drivers will allow small number of queues for each queue family,
        // and we'll create all the command-buffers on multiple threads and
        // submit them all at once on the main thread, so we just need one perthread.
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // 2. Specify used device features, the features that we queried previously
    // using 'VkPhysicalDeviceFeatures' (e.g. geometry shaders)
    VkPhysicalDeviceFeatures deviceFeatures{};

    // 3. Create the logical device
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos          = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount       = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pEnabledFeatures           = &deviceFeatures;

    // device extensions
    deviceCreateInfo.enabledExtensionCount      = static_cast<uint32_t>(m_deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames    = m_deviceExtensions.data();

    // notice that we've already set the extension & validation layers for VkInstance.
    // we do the same thing for the physical device, which is actually not necessary
    // because Vulkan will automatically pick the up-to-date info, but it's good to 
    // specify anyways.
    if (enableValidationLayers)
    {
        deviceCreateInfo.enabledLayerCount   = static_cast<uint32_t>(m_validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();
    }
    else
        deviceCreateInfo.enabledLayerCount = 0;

    // 4. Instantiate logical device!
    if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan logical device");
        return false;
    }

    // 5. Retrieve queue handles (here, graphics queue)
    // Normally, the queues are automatically created along with the logical device,
    // but we need a handler to interface with them.
    const uint32_t k_queueIndex = 0;    // since we know that we only have one family...
    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), k_queueIndex, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentationFamily.value(), k_queueIndex, &m_presentationQueue);

    PRINTLN("Created logical device");
    return true;
}


// ----------------------<<  Window Surface  >>-----------------------------
//
//  In order for Vulkan to establish a connection with a platform specific
//  window system, we need to use the Window System Integration (WSL)
//  extensions.
//
// -------------------------------------------------------------------------

bool VulkanManager::createWindowSurface(GLFWwindow* window)
{
    // Normally, you would create an Vulkan object for surface creation
    // (eg. VkWin32SurfaceCreateInfoKHR createInfo{} ...)
    // however, glfw automatically handles this.

    PRINT_BAR_DOTS();

    if (glfwCreateWindowSurface(m_VkInstance, window, nullptr, &m_windowSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface");
        return false;
    }

    PRINTLN("Created Vulkan window surface");
    return true;
}


// -------------------<<  Swap Chain Support  >>-----------------------
//
//  Simply checking whether swapchain is available from the device is
//  not sufficient. Creating a swapchain involves a lot of configuration
//  therefore needs many queries.
//
// --------------------------------------------------------------------

SwapchainSupportDetails VulkanManager::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapchainSupportDetails details;

    // 1. Surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_windowSurface, &details.surfaceCapabilities);

    // 2. Surface formats
    uint32_t n_formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_windowSurface, &n_formatCount, nullptr);
    if (n_formatCount != 0)
    {
        details.formats.resize(n_formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_windowSurface, &n_formatCount, details.formats.data());
    }

    // 3. Presentation modes
    uint32_t n_presentationModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_windowSurface, &n_presentationModeCount, nullptr);
    if (n_presentationModeCount != 0)
    {
        details.presentModes.resize(n_presentationModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_windowSurface, &n_presentationModeCount, details.presentModes.data());
    }

    return details;
}


// --------------------------<<  Exit  >>----------------------------
//
//  VkPhysicalDevice - automatically handled
//
// ------------------------------------------------------------------

void VulkanManager::cleanVulkan()
{
    // extensions must be destroyed before vulkan instance
    if (enableValidationLayers)
        destroyDebugUtilsMessengerEXT(m_VkInstance, &m_debugMessenger, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_VkInstance, m_windowSurface, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);
    
    PRINTLN("Cleaned up Vulkan");
}
