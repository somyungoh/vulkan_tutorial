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
#include <cstdint>  // UINT32_MAX
#include <algorithm>    // std::min, std::max


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

    result &= createSwapChain(window);
    result &= createImageViews();

    result &= createGraphicsPipeline();

    PRINT_BAR_DOTS();
    if (result)
        PRINTLN("Successfully initialized Vulkan Manager");
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

bool VulkanManager::createSwapChain(GLFWwindow* window)
{
    PRINT_BAR_DOTS();

    SwapchainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice);

    // load main swapchain settings
    VkSurfaceFormatKHR  surfaceFormat   = chooseSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR    presentMode     = choosePresentMode(swapChainSupport.presentModes);
    VkExtent2D          extent          = chooseExtent2D(window, swapChainSupport.surfaceCapabilities);

    // minimum #. images to have in the swap chain.
    // it is better to a little more than the minimum number, since sometimes
    // we need to wait for the driver to complete the internal operations before
    // aquiring the next image to render.
    uint32_t n_ImageCount = swapChainSupport.surfaceCapabilities.minImageCount + 1;
    if (swapChainSupport.surfaceCapabilities.maxImageCount > 0 &&
        n_ImageCount > swapChainSupport.surfaceCapabilities.maxImageCount)
        n_ImageCount = swapChainSupport.surfaceCapabilities.maxImageCount;

    // create swapchain object
    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType               = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface             = m_windowSurface;
    swapchainCreateInfo.minImageCount       = n_ImageCount;
    swapchainCreateInfo.imageFormat         = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace     = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent         = extent;
    swapchainCreateInfo.imageArrayLayers    = 1;    // # of layers of each image. Always 1 unless it's stereoscopic 3D.
    // what kind of operations that images will used in the swapchain for.
    // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT - render directly
    // VK_IMAGE_USAGE_TRANSFER_DST_BIT - render separately (i.e need to use this for post-processing)
    swapchainCreateInfo.imageUsage          = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


    // Specify how to handle swapchain images that will be used across multiple queue families.
    // Here, we draw in swapcahin from graphics queue, then submit to the presentation queue.
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentationFamily.value()};

    // There are two ways of handling the images that is accessed by multiple queues.
    // VK_SHARING_MODE_EXCLUSIVE: Image owned by one queue family, an explicit transfer
    //                            is required before used by another queue family. Best in performance.
    // VK_SHARING_MODE_CONCURRENT: Images can be used across different queue families without
    //                             explicit ownership.
    if (indices.graphicsFamily != indices.presentationFamily)
    {
        // for simplicity, we let it to use concurrent mode when the queue family is different.
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        // exclusive mode is ideal in most of the hardwares if the queue is the same.
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;  // optional
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;  // optional
    }

    // transformation capability
    // such as 90 clockwise rotation, flip... set to current transform if you don't want any
    swapchainCreateInfo.preTransform = swapChainSupport.surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;     // ignore alpha channel

    // as it says.
    swapchainCreateInfo.presentMode = presentMode;

    // clip (do not draw) if the pixel is covered by another window
    swapchainCreateInfo.clipped = VK_TRUE;  // clip it

    // A bit complex topic, in case the program fails and need to create the swapchain
    // again from scratch, should Vulkan should keep the old swapchain as a reference.
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;  // no. only create one swapchain ever.


    // Finally, create swapchain
    if (vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain) != VK_SUCCESS)
        throw std::runtime_error("Failed to create swapcahin!");

    // retr ieve swapchain images
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &n_ImageCount, nullptr);
    m_swapchainImages.resize(n_ImageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &n_ImageCount, m_swapchainImages.data());

    m_swapchainImageFormat = surfaceFormat.format;
    m_swapchainExtent = extent;

    PRINTLN("Created Swap Chain");

    return true;
}

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

VkSurfaceFormatKHR VulkanManager::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    // "VkSurfaceFormatKHR" defines - pixelformat, colorspace.
    for (const auto& availableFormat : availableFormats)
    {
        // here, determine which pixelformat / colorspace to choose
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&                // 8-bit BRGA
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)    // sRGB
        {
            PRINTLN("Swapchain) Choose surface format \"8bit-BRGA pixelformat\" & \"sRGB colorspace\"");
            return availableFormat;
        }
    }

    PRINTLN("Swapchain) Couldn't choose a good format. Returning first available surface format");
    return availableFormats[0];
}

VkPresentModeKHR VulkanManager::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    // Choosing a presentation mode is arguably most important setting.
    // It represents the actual conditions for showing the images for
    // showing the images on the screen.
    // 1) VK_PRESENT_MODE_IMMEDIATE_KHR: draw submitted images immediately
    // 2) VK_PRESENT_MODE_FIFO_KHR: draw image one in the front queue
    // 3) VK_PRESENT_MODE_FIFO_RELAXED_KHR: 1) + 2)
    // 4) VK_PRESENT_MODE_MAILBOX_KHR: replace existing queue element with a
    //    new one at arrival if the queue is full. AKA "Triple Buffering"

    for (const auto& availablePresentMode : availablePresentModes)
    {
        // here choose your own taste
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            PRINTLN("Swapchain) Choosing \"VK_PRESENT_MODE_MAILBOX_KHR\" for presentation mode");
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }

    PRINTLN("Swapchain) Choosing \"VK_PRESENT_MODE_FIFO_KHR\" for presentation mode");
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanManager::chooseExtent2D(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities)
{
    // This determines the resolution of the swap chain images.
    // Most of the cases, this is almost always exactly equal to the resolution
    // of the window that we're drawing to in pixels.

    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width  = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.width, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }
}


// -----------------------<<  Image Views  >>------------------------
//
//  Using VkImage requires VkImageView, which is literally a "view"
//  into the image. It describes how & which part of the image to
//  access (i.e. 2D Depth Texture, no mipmap levels?)
//
// ------------------------------------------------------------------

bool VulkanManager::createImageViews()
{
    // 1. resize the array into the size of our needs. As of now,
    // the only VkImage we have is in the swapchain.
    m_swapchainImageViews.resize(m_swapchainImages.size());

    for (int i = 0; i < m_swapchainImageViews.size(); i++)
    {
        // initialize each slots
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image       = m_swapchainImages[i];
        imageViewCreateInfo.viewType    = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format      = m_swapchainImageFormat;
        // [components] field allows to swizzle the color channels (i.e. map
        // all colors to the red channel), but we stick to the default.
        imageViewCreateInfo.components.r    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a    = VK_COMPONENT_SWIZZLE_IDENTITY;
        // [subresourceRange] field defines the image's purpose and which
        // part of the image is accessed.
        imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;    // used as color targets
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;    // no mipmaps
        imageViewCreateInfo.subresourceRange.levelCount     = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount     = 1;

        // create image view
        VkResult result = vkCreateImageView(m_device,
                                            &imageViewCreateInfo,
                                            nullptr,
                                            &m_swapchainImageViews[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
            return false;
        }
    }

    PRINTLN("Created image views");
    return true;
}

// ----------------------<<  Graphics Pipepline  >>--------------------------
//
//  Graphics Pipeline: from vertex/textures to pixels in the Render Targets.
//  - Input Assembler) collect raw vertex data from the buffer
//  - Vertex Shader) model - view transformtation for all vertices
//  - Tesselation) geometry subdivision for a better rendering quality
//  - Geometry Shader) primitive (point,line,triangle) drop/copy
//      * normally does not have good performance on external GPUs
//  - Rasterization) primitives to fragments(pixels), filled in frame buffers
//  - Fragment Shader) determine final colors of each frame buffers
//  - Color Blending) mixing different frame buffers
//
// --------------------------------------------------------------------------

bool VulkanManager::createGraphicsPipeline()
{
    return true;
};


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
    for (auto imageView : m_swapchainImageViews)
        vkDestroyImageView(m_device, imageView, nullptr);
    // swapchain is no exception
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_VkInstance, m_windowSurface, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);

    PRINTLN("Cleaned up Vulkan");
}
