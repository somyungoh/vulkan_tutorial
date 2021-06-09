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
    createVulkanInstance();
    createDebugMessenger();
    createWindowSurface(window);
    loadPhysicalDevice();
    createLogicalDevice();
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

    if (candidates.rbegin()->first > 0 && isDeviceSuitable(candidates.rbegin()->second))
        m_physicalDevice = candidates.rbegin()->second;
    else if (m_physicalDevice == VK_NULL_HANDLE)
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

bool VulkanManager::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionSupported = checkDeviceExtensionSupport(m_physicalDevice);

    return indices.isComplete() && extensionSupported;
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
        requiredExtensions.erase(extension.extensionName);

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
    PRINT_BAR();

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
        PRINTLN_VERBOSE("Graphics Queue family indices: " << indices.graphicsFamily.value());

    return indices;
}


// ----------------------<<  Logical Device  >>-----------------------------
//
//  After selecting a physical device, we need to setup a 'logical device'
//  to interface with it. Here, we specify which queues to create from
//  the queue family that we quried previously.
//
// -------------------------------------------------------------------------

void VulkanManager::createLogicalDevice()
{
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
        throw std::runtime_error("Failed to create Vulkan logical device");

    // 5. Retrieve queue handles (here, graphics queue)
    // Normally, the queues are automatically created along with the logical device,
    // but we need a handler to interface with them.
    const uint32_t k_queueIndex = 0;    // since we know that we only have one family...
    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), k_queueIndex, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentationFamily.value(), k_queueIndex, &m_presentationQueue);
}


// ----------------------<<  Window Surface  >>-----------------------------
//
//  In order for Vulkan to establish a connection with a platform specific
//  window system, we need to use the Window System Integration (WSL)
//  extensions.
//
// -------------------------------------------------------------------------

void VulkanManager::createWindowSurface(GLFWwindow* window)
{
    // Normally, you would create an Vulkan object for surface creation
    // (eg. VkWin32SurfaceCreateInfoKHR createInfo{} ...)
    // however, glfw automatically handles this.
    if (glfwCreateWindowSurface(m_VkInstance, window, nullptr, &m_windowSurface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");
}


// --------------------------<<  Exit  >>---------------------------
//
//  VkPhysicalDevice - automatically handled
//
// ------------------------------------------------------------------

void VulkanManager::cleanVulkan()
{
    // extensions must be destroyed before vulkan instance
    if (enableValidationLayers)
        destroyDebugUtilsMessengerEXT(m_VkInstance, &m_debugMessenger, nullptr);
    vkDestroySurfaceKHR(m_VkInstance, m_windowSurface, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);
    vkDestroyDevice(m_device, nullptr);
    
    PRINTLN("Successfully cleaning up Vulkan...");
}
