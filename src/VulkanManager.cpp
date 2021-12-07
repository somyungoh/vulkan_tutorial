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
#include <fstream>

#ifndef MAX_FRAMES_IN_FLIGHT
#   define MAX_FRAMES_IN_FLIGHT 2
#endif

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

static std::vector<char> readFile(const std::string& filename)
{
    // ate: start reading at the end, binary: read as binary file
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    PRINTLN("open file - " + filename);

    if (!file.is_open())
        throw std::runtime_error("failed to open file - " + filename + "");

    // as we read at the end, we can instantly determine the size of the file'
    size_t filesize = (size_t) file.tellg();
    std::vector<char> buffer(filesize);

    // go back to the beginning and read all at once
    file.seekg(0);
    file.read(buffer.data(), filesize);

    file.close();

    return buffer;
}


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
    m_deviceExtensions({ VK_KHR_SWAPCHAIN_EXTENSION_NAME }),
    m_curretFrameIndex(0),
    m_frameBufferResized(false)
    {
    };


void VulkanManager::initVulkan(GLFWwindow* window)
{
    PRINT_BAR_LINE();
    PRINTLN("Start initializing vulkan manager.");

    m_window = window;

    bool result = true;

    // Initial Setup
    result &= createVulkanInstance();
    result &= createDebugMessenger();
    PRINT_BAR_DOTS();

    // Presentation
    result &= createWindowSurface();
    result &= loadPhysicalDevice();
    result &= createLogicalDevice();
    PRINT_BAR_DOTS();
    result &= createSwapChain();
    result &= createImageViews();
    PRINT_BAR_DOTS();

    // Graphics Pipeline
    result &= createRenderPass();
    result &= createGraphicsPipeline();
    PRINT_BAR_DOTS();

    // Drawing
    result &= createFrameBuffers();
    result &= createCommandPool();
    result &= createCommandBuffers();
    PRINT_BAR_DOTS();

    // Rendering & Presentation
    result &= createSyncObjects();

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

bool VulkanManager::createWindowSurface()
{
    // Normally, you would create an Vulkan object for surface creation
    // (eg. VkWin32SurfaceCreateInfoKHR createInfo{} ...)
    // however, glfw automatically handles this.

    if (glfwCreateWindowSurface(m_VkInstance, m_window, nullptr, &m_windowSurface) != VK_SUCCESS)
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

bool VulkanManager::createSwapChain()
{
    SwapchainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice);

    // load main swapchain settings
    VkSurfaceFormatKHR  surfaceFormat   = chooseSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR    presentMode     = choosePresentMode(swapChainSupport.presentModes);
    VkExtent2D          extent          = chooseExtent2D(swapChainSupport.surfaceCapabilities);

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

bool VulkanManager::recreateSwapChain()
{
    // Swapchain information can be outdated such when window size has changed.
    // In that case, we will need to create a new swapchain.

    // handle window minimized case, in which the framebuffer size is 0.
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_device);

    cleanSwapChain();

    bool result = true;
    result &= createSwapChain();
    result &= createImageViews();
    result &= createRenderPass();
    result &= createGraphicsPipeline();
    result &= createFrameBuffers();
    result &= createCommandBuffers();

    return result;
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

VkExtent2D VulkanManager::chooseExtent2D(const VkSurfaceCapabilitiesKHR& capabilities)
{
    // This determines the resolution of the swap chain images.
    // Most of the cases, this is almost always exactly equal to the resolution
    // of the window that we're drawing to in pixels.

    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    else
    {
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);

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

    PRINTLN("Created Image Views");
    return true;
}

// -------------------------<<  Render Passes  >>----------------------------
//
//  Render pass contains information about the framebuffer attachments
//  that will be used during the rendering, such as:
//  number of colors, depth buffers, how many samples for each of them
//  how their contents should be handled...
//
// --------------------------------------------------------------------------

bool VulkanManager::createRenderPass()
{
    // 1. Attachment description
    VkAttachmentDescription colorAttachmentDescription{};
    colorAttachmentDescription.format   = m_swapchainImageFormat;   // should match with swapchain images
    colorAttachmentDescription.samples  = VK_SAMPLE_COUNT_1_BIT;    // no multi-sample
    // these two speficies what to do with the data attachment before/after loading
    //  [loadOP]
    //      _LOAD: preserve the existing contents of the attachment
    //      _CLEAR: clear the values to a constant before start
    //      _DONT_CARE: existing contents are undefined, we don't care about them
    //  [storeOp]
    //      _STORE: store in memory so it can be read later
    //      _DONT_CARE: contents of the framebuffer will remain undefined
    colorAttachmentDescription.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDescription.stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // the textures and framebuffers are represented by VkImage objects in certain pixel formats,
    // however this can be changed in the middle based on what you are trying to do.
    //  _COLOR_ATTACHMENT_OPTIMAL: used for color attachment
    //  _PRESENT_SRC_KHR: presented in the swapchain
    //  _TRANSFER_DST_OPTIMAL: used for memory copy operation
    colorAttachmentDescription.initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.finalLayout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // 2. Subpasses
    // All subpasses references one or more VkAttachmentDescription
    VkAttachmentReference colorAttachmentReference{};
    colorAttachmentReference.attachment  = 0;    // attachment index. our case, we only have one, therefore 0
    colorAttachmentReference.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // we want layout as color buffer, in best optimized

    VkSubpassDescription subpassDescription{};
    subpassDescription.pipelineBindPoint     = VK_PIPELINE_BIND_POINT_GRAPHICS; // in case vulkan supports compute subpasses
    subpassDescription.colorAttachmentCount  = 1;   // index of this array is directly referenced by the fragment shader via layout(location = 0)
    subpassDescription.pColorAttachments     = &colorAttachmentReference;

    // This specifies memory and execution dependencies between subpasses
    // including the start/end. Although, these have built-in dependencies.
    VkSubpassDependency subpassDependency{};
    // indices of the dependency and the dependent subpass
    subpassDependency.srcSubpass    = VK_SUBPASS_EXTERNAL;  // before/after the renderpass
    subpassDependency.dstSubpass    = 0;    // first pass (the only one is our case)
    // operation to wait and in which stage that occurs
    // here, we wait for swapchain to finish reading the image before we access it
    subpassDependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;
    // color attatchment write - will wait until the "dstStageMask" stage
    subpassDependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // 3. Render Passes
    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount    = 1;
    renderPassCreateInfo.pAttachments       = &colorAttachmentDescription;
    renderPassCreateInfo.subpassCount       = 1;
    renderPassCreateInfo.pSubpasses         = &subpassDescription;
    renderPassCreateInfo.dependencyCount    = 1;
    renderPassCreateInfo.pDependencies      = &subpassDependency;

    if (vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
        return false;
    }

    PRINTLN("Created Render Passes");

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
    // 1. Load shaders
    auto vertShader = readFile("../src/shaders/vert.spv");
    auto fragShader = readFile("../src/shaders/frag.spv");

    // 2. Create shader moduless
    VkShaderModule vertShaderModule = createShaderModule(vertShader);
    VkShaderModule fragShaderModule = createShaderModule(fragShader);

    // 3. Stage shaders into the pipeline stage
    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
    vertShaderStageCreateInfo.sType     = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage     = VK_SHADER_STAGE_VERTEX_BIT;   // vertex shader
    vertShaderStageCreateInfo.module    = vertShaderModule;
    vertShaderStageCreateInfo.pName     = "main";   // entry point
    // not using in this code, but we can also pass optional "SpecializationInfo"
    // specifying different shader constant values. using this is faster than using
    // shader variables in runtime, becuase the compiler can elimitate (e.g. using if)
    // stuff at compile time.
    // vertShaderStageCreateInfo.pSpecializationInfo = // Shader Constants
    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
    fragShaderStageCreateInfo.sType     = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage     = VK_SHADER_STAGE_FRAGMENT_BIT;   // fragment shader
    fragShaderStageCreateInfo.module    = fragShaderModule;
    fragShaderStageCreateInfo.pName     = "main";   // entry point

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};


    // 4. Fixed Functions
    //    usually these were set with default values in other GraphicsAPI, but not for Vulkan, so...

    // 4.1 Vertex input
    // we hard-coded these in the shader, so we'll leave 'none' for now
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType                            = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount    = 0;
    vertexInputStateCreateInfo.pVertexBindingDescriptions       = nullptr;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount    = 0;
    vertexInputStateCreateInfo.pVertexAttributeDescriptions     = nullptr;

    // 4.2 Input Aseembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
    inputAssemblyStateCreateInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.topology                   = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;    // topology (point, line... *_LIST: vertex is reused)
    inputAssemblyStateCreateInfo.primitiveRestartEnable     = VK_FALSE;     // setting this to true + using _STRIP topology allows you to break up lines & triangles

    // 4.3 Viewport
    // the region of the framebuffer that will be rendered out. Almost always (0,0) ~ (width, height)
    VkViewport viewport{};
    viewport.x          = 0.0f;
    viewport.y          = 0.0f;
    viewport.width      = (float) m_swapchainExtent.width;
    viewport.height     = (float) m_swapchainExtent.height;
    viewport.minDepth   = 0.0f; // range of depth value in the framebuffer.
    viewport.maxDepth   = 1.0f; // always within (0,1), but min value can be greater than max

    // 4.4 Scissors
    // define in which regions of pixels will be rendered, then it will be discarded by the rasterizer
    VkRect2D scissor{};
    scissor.offset = {0, 0};    // cover from the beginning
    scissor.extent = m_swapchainExtent; // to full size of the swapchain

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount   = 1;
    viewportStateCreateInfo.pViewports      = &viewport;
    viewportStateCreateInfo.scissorCount    = 1;
    viewportStateCreateInfo.pScissors       = &scissor;

    // 4.5 Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.depthClampEnable           = VK_FALSE; // clamp range beyond near/far plane
    rasterizationStateCreateInfo.rasterizerDiscardEnable    = VK_FALSE; // don't let geometry to pass the rasterizer, and won't display on framebuffer
    rasterizationStateCreateInfo.polygonMode                = VK_POLYGON_MODE_FILL; // _LINE / _POINT (these two requires GPU feature)
    rasterizationStateCreateInfo.lineWidth                  = 1.0f; // thickness of lines covering the fragment. Larger than 1.0 requires GPU feature
    rasterizationStateCreateInfo.cullMode                   = VK_CULL_MODE_BACK_BIT;    // back-face culling
    rasterizationStateCreateInfo.frontFace                  = VK_FRONT_FACE_CLOCKWISE;  // vertex oreder of the front face
    rasterizationStateCreateInfo.depthBiasEnable            = VK_FALSE; // set true - adjust depth values through the values below
    rasterizationStateCreateInfo.depthBiasConstantFactor    = 0.0f;
    rasterizationStateCreateInfo.depthBiasClamp             = 0.0f;
    rasterizationStateCreateInfo.depthBiasSlopeFactor       = 0.0f;

    // 4.6 Multisampling
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable      = VK_FALSE;
    multisampleStateCreateInfo.rasterizationSamples     = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateCreateInfo.minSampleShading         = 1.0f; // optional
    multisampleStateCreateInfo.pSampleMask              = nullptr;  // optional
    multisampleStateCreateInfo.alphaToCoverageEnable    = VK_FALSE; // optional
    multisampleStateCreateInfo.alphaToOneEnable         = VK_FALSE; // optional
    // will come back.

    // 4.7 Depth/Stencil buffers
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
    depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // to be continued.

    // 4.8 Color blending
    // This happens when output of the fragment shader needs to be combined with the
    // color that already exists in the frame buffer.
    // You can do this in two ways - mix the two, or combine with bitwise operation

    // contains per-framebuffer configuration
    VkPipelineColorBlendAttachmentState colorblendAttachmentState{};
    colorblendAttachmentState.colorWriteMask        = VK_COLOR_COMPONENT_R_BIT |    // colors that is actually passed through
                                                      VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorblendAttachmentState.blendEnable           = VK_TRUE;
    // below is optional, it's a sample config for alpha blending c1(a) * c2(1-a)
    colorblendAttachmentState.srcColorBlendFactor   = VK_BLEND_FACTOR_SRC_ALPHA;
    colorblendAttachmentState.dstAlphaBlendFactor   = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorblendAttachmentState.colorBlendOp          = VK_BLEND_OP_ADD;
    colorblendAttachmentState.srcAlphaBlendFactor   = VK_BLEND_FACTOR_ONE;
    colorblendAttachmentState.dstAlphaBlendFactor   = VK_BLEND_FACTOR_ZERO;
    colorblendAttachmentState.alphaBlendOp          = VK_BLEND_OP_ADD;

    // This references array of structures for all the framebuffers, and set blend constants
    // that you can use it as a blend factors.
    VkPipelineColorBlendStateCreateInfo colorblendStateCreateInfo{};
    colorblendStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorblendStateCreateInfo.logicOpEnable     = VK_FALSE;             // enable this to use the second method (bitwise operation)
    colorblendStateCreateInfo.logicOp           = VK_LOGIC_OP_COPY;     // optional
    colorblendStateCreateInfo.attachmentCount   = 1;
    colorblendStateCreateInfo.pAttachments      = &colorblendAttachmentState;
    colorblendStateCreateInfo.blendConstants[0] = 0.0f; // optional
    colorblendStateCreateInfo.blendConstants[1] = 0.0f; // optional
    colorblendStateCreateInfo.blendConstants[2] = 0.0f; // optional
    colorblendStateCreateInfo.blendConstants[3] = 0.0f; // optional

    // 4.9 Dynamic state
    // certain states can be changed without creating a whole new pipeline state (e.x viewport size, blend constants...)
    // simply fill the VkDynamicState structure. As a result, these value will be ignored at first
    // and required to be specify the data during the draw.
    VkDynamicState dynamicState[] {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount    = 2;
    dynamicStateCreateInfo.pDynamicStates       = dynamicState;

    // 4.10 Pipeline layout
    // this allows you to pass 'uniform' constants to the shaders.
    // In practice, transform matrices are usually passed through this.
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount         = 0;    // optional
    pipelineLayoutCreateInfo.pSetLayouts            = nullptr;  // optional
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;    // optional
    pipelineLayoutCreateInfo.pPushConstantRanges    = nullptr;  // optional

    // this is a manatory field to register even though we leave blank, so
    bool result = false;
    if (vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create pipeline layout!");
    else
    {
        result = true;
        PRINTLN("Created Graphics Pipeline Layout");
    }

    // 5. Graphics Pipeline
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.sType                = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount           = 2;    // vertex, fragment shader
    graphicsPipelineCreateInfo.pStages              = shaderStages;

    graphicsPipelineCreateInfo.pVertexInputState    = &vertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState  = &inputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pViewportState       = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState  = &rasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState    = &multisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState   = nullptr;  // optional
    graphicsPipelineCreateInfo.pColorBlendState     = &colorblendStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState        = nullptr;  // optional

    graphicsPipelineCreateInfo.layout               = m_pipelineLayout;
    graphicsPipelineCreateInfo.renderPass           = m_renderPass;
    graphicsPipelineCreateInfo.subpass              = 0;    // index of the subpass

    // vulkan allows to create a new pipeline from an existing pipeline, as this is
    // cheaper than creating a whole entire new one. We are not using it here,
    // but you can set to VK_PIPELINE_CREATE_DERIVATIVE_BIT to activate it.
    graphicsPipelineCreateInfo.basePipelineHandle   = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.basePipelineIndex    = -1;

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo,
                                  nullptr, &m_graphicsPipeline)
        != VK_SUCCESS)
        throw std::runtime_error("failed to creat graphics pipeline!");
    else
        result &= true;

    // shader module cleanup
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);

    PRINTLN("Created Graphics Pipeline");

    return result;
};

VkShaderModule VulkanManager::createShaderModule(const std::vector<char> &code)
{
    // this method creates vulkan shader module from the shader byte code

    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType        = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize     = code.size();
    // the only thing we need to for ShaderModule is to specify a pointer to the
    // shader byte code buffer. We stored the byte code in char however the pointer
    // type is u_int32_t*, so we need to type cast. Normally you need to be careful
    // with the data alignment but in our case std::vetcor does the job already.
    shaderModuleCreateInfo.pCode        = reinterpret_cast<const u_int32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device, &shaderModuleCreateInfo, nullptr, &shaderModule)
        != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");

    return shaderModule;
}


// -------------------------<<  Frame Buffers  >>---------------------------------
//
//  Previously we created render passes expecting to have a framebuffer with the
//  same format as the swapchain images. It's time to create one.
//
//  The attachments that we specified in the render pass is wrapped into a
//  Framebuffer object "VkFrameBuffer", which references to all the VkImageView
//  objects that represents the attachments.
//
//  We need to create framebuffers for all corresponding swapchain images.
//
// -------------------------------------------------------------------------------

bool VulkanManager::createFrameBuffers()
{
    // resize as same as the swapchain imageViews
    m_swapchainFrameBuffers.resize(m_swapchainImageViews.size());

    // iterate every imageViews and create a corresponding frame buffer
    bool result = true;
    for (size_t i = 0; i < m_swapchainFrameBuffers.size(); i++)
    {
        VkImageView attachments[] = {m_swapchainImageViews[i]};

        VkFramebufferCreateInfo frameBufferCreateInfo{};
        frameBufferCreateInfo.sType             = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferCreateInfo.renderPass        = m_renderPass; // compatible render pass
        // VkImageView objects to reference
        frameBufferCreateInfo.attachmentCount   = 1;    // in our case, just color attachment
        frameBufferCreateInfo.pAttachments      = attachments;
        frameBufferCreateInfo.width             = m_swapchainExtent.width;
        frameBufferCreateInfo.height            = m_swapchainExtent.height;
        // number of layers in image arrays.
        frameBufferCreateInfo.layers            = 1;    // our swapchain images only has a single image

        if (vkCreateFramebuffer(m_device, &frameBufferCreateInfo, nullptr, &m_swapchainFrameBuffers[i])
            != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
            result = false;
        }
    }

    if (result == true)
        PRINTLN("Created Frame Buffers");

    return result;
}


// ------------------------<<  Command Buffers  >>---------------------------
//
//  Commnads, here includes such as drawing operations, memory transfers.
//  In Vulkan, you need to record all these commands in the command buffer
//  objects. It's a hard work than other APIs but there an advantage of
//  being able to setting up the commands all in advance and simply have to
//  execute in the main loop.
//
// --------------------------------------------------------------------------

bool VulkanManager::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    // command buffers are executed by submitting them to one of the device queues.
    // (recap that we had graphics queues, presentation queues...)
    commandPoolCreateInfo.queueFamilyIndex  = queueFamilyIndices.graphicsFamily.value();    // chose graphics queue to store drawing commands
    // optional flag has two choices:
    //  - VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: command buffers are recorded with new commands very often
    //  - VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: allow command buffers to be recoreded individually
    commandPoolCreateInfo.flags             = 0;    // neither in that case

    if (vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
        return false;
    }

    PRINTLN("Created Command Pool");

    return true;
}

bool VulkanManager::createCommandBuffers()
{
    // Here we record the drawing commands, which requires to bind into a correct VkFrameBuffer.
    // Therefore, we'll record the command for each swapchain images into the VkCommandBuffer objects.
    // Note that these command objects will be freed when the command pool is destroyed, so
    // no explicit clean up is required.

    bool result = true;

    m_commandBuffers.resize(m_swapchainFrameBuffers.size());

    VkCommandBufferAllocateInfo commandBufferAllocationInfo{};
    commandBufferAllocationInfo.sType           = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocationInfo.commandPool     = m_commandPool;
    // level parameter specifies whether the command is a primary or secondary buffer:
    //  - _LEVEL_PRIMARY: can be submitted for execution, but cannot be called from other command buffers
    //  - _LEVEL_SECONDARY: cannoy be submitted directly, but can be called from the primary command buffers
    commandBufferAllocationInfo.level               = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocationInfo.commandBufferCount  = (uint32_t)m_commandBuffers.size();

    if (vkAllocateCommandBuffers(m_device, &commandBufferAllocationInfo, m_commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
        result = false;
    }

    for (size_t i = 0; i < m_commandBuffers.size(); i++)
    {
        // 1. Start recording command buffers
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // optional flag specifying how the command buffers will be used
        //  - VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: will be recorded right after executing it once
        //  - VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: will be a secondary command buffer living in a single render pass
        //  - VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: will be able to be re-submitted even if it's in a pending state
        commandBufferBeginInfo.flags            = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;  // only relavent for secondary command buffers

        if (vkBeginCommandBuffer(m_commandBuffers[i], &commandBufferBeginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin command buffer!");
            result = false;
        }

        // 2. Start render passes
        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass  = m_renderPass;
        renderPassBeginInfo.framebuffer = m_swapchainFrameBuffers[i];
        // render area size
        renderPassBeginInfo.renderArea.offset   = {0, 0};
        renderPassBeginInfo.renderArea.extent   = m_swapchainExtent;
        // clear color that will be used by VK_ATTACHMENT_LOAD_OP_CLEAR option we previouly set
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues    = &clearColor;

        // Begin render pass
        // last parameter: how the drawing command within the render pass will be provided.
        //  - VK_SUBPASS_CONTENTS_INLINE: render pass commands will be embedded in the primary commnad buffer, no secondary command buffer execution happening
        //  - VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: render pass commands are executed in the secondary command buffers
        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Bind graphics pipeline
        vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);    // graphics or compute?

        // 3. Record commands
        // All the functions that record commands are prefixed with vkCmd
        // (vertex count, instance count, first vertex, first instance)
        vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0);

        // 4. Finish
        vkCmdEndRenderPass(m_commandBuffers[i]);
        if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
            result = false;
        }
    }

    if (result == true)
        PRINTLN("Created Command Buffers");

    return result;
}

void VulkanManager::setFrameBufferResized(bool isResized)
{
    m_frameBufferResized = isResized;
}

// ---------------------<<  Rendering & Presentation  >>----------------------
//
//  Series of rendering operations - swapchain image inquiry, commnad buufer
//  execution, returning image to the swapchain - are done asyncronously.
//
// ---------------------------------------------------------------------------

bool VulkanManager::createSyncObjects()
{
    // Syncronizing swapchain events can be done in two ways - Fences or
    // Semaphores.
    // "Fences" states can be accessed from the program using "vkWaitForFences"
    // and is mainly for the purpose of syncronizing the application with the
    // rendering.
    // "Semaphores" in the other hand, cannot be accessed by the program and it's
    // usage is mainly for syncronizing across the command queues.

    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);    // for command queue syncronization
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);    // for command queue syncronization
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);  // for CPU-GPU syncronization
    m_imagesInFlight.resize(m_swapchainImages.size(), VK_NULL_HANDLE);  // track images in flight

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores and fences!");
            return false;
        }
    }

    PRINTLN_VERBOSE("Created Semaphores");

    return true;
}

bool VulkanManager::acquireNextImageIndex(const uint32_t frameIndex, uint32_t &nextImageIndex)
{
    // Returns whether the swapchain is still adequate for the presentation
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSemaphores[frameIndex],
                                            VK_NULL_HANDLE, &nextImageIndex);

    // Possible returns:
    // VK_ERROR_OUT_OF_DATE_KHR: swapchain became incompatible with the surface and can no longer be used.
    //                           usually happens due to window resizing.
    //  VK_SUBOPTIMAL_KHR: swapchain can be still used but the properties no longer match.
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
        recreateSwapChain();
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to aquire swapchain images!");
        return false;
    }

    return true;
}

bool VulkanManager::submitCommandBuffer(const uint32_t frameIndex, const uint32_t imageIndex)
{
    VkSemaphore             signalSemaphores[] = {m_renderFinishedSemaphores[frameIndex]};
    VkSemaphore             waitSemaphores[] = {m_imageAvailableSemaphores[frameIndex]};
    VkPipelineStageFlags    waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo{};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // Semaphore information. In this case, we would like to wait until the
    // graphics pipeline stage where color attachment is available.
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.pWaitDstStageMask    = waitStages;
    // Which command buffer to submit. It should be binded into the
    // acquired swapchain image
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &m_commandBuffers[imageIndex];
    // Which semaphores to signal once command buffer has finised execution
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[frameIndex]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit command buffer!");
        return false;
    }

    return true;
}

bool VulkanManager::submitPresentation(const uint32_t frameIndex, const uint32_t imageIndex)
{
    // This is the last step for display something on the screen,
    // is to submit the image back to the swapchain.
    VkSemaphore     signalSemaphores[] = {m_renderFinishedSemaphores[frameIndex]};
    VkSwapchainKHR  swapchains[] = {m_swapchain};

    VkPresentInfoKHR presentationInfo{};
    presentationInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentationInfo.waitSemaphoreCount = 1;
    presentationInfo.pWaitSemaphores    = signalSemaphores;
    // which swapchain to present images and it's image index
    presentationInfo.swapchainCount     = 1;
    presentationInfo.pSwapchains        = swapchains;
    presentationInfo.pImageIndices      = &imageIndex;
    // array of VK_RESULT that corresponds to each swapchain images
    presentationInfo.pResults           = nullptr;

    VkResult result =  vkQueuePresentKHR(m_presentationQueue, &presentationInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_frameBufferResized)
    {
        recreateSwapChain();
        m_frameBufferResized = false;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit presentation info!");
        return false;
    }


    return true;
}

void VulkanManager::drawFrame()
{
    m_curretFrameIndex = (m_curretFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
    uint32_t imgIndex;
    acquireNextImageIndex(m_curretFrameIndex, imgIndex);

    // CPU - GPU syncronization.
    // Normally at this point, GPU work speed cannot follow up the CPU work
    // submission speed, ending up submission queue growing by time. Validation
    // Layer raises an error or warning about this if enbled.
    // There are two ways you can handle this:
#if 0
    // Quick & lazy way
    vkQueueWaitIdle(m_presentationQueue);   // no need of any fences
#else
    // Frames in Flight
    // check if the previous image is using this image
    if (m_imagesInFlight[imgIndex] != VK_NULL_HANDLE)
        // Wait for any or all of the passed fences (VK_TRUE means wait for ALL of them).
        vkWaitForFences(m_device, 1, &m_imagesInFlight[imgIndex], VK_TRUE, UINT64_MAX);
    // mark current frame is using this image
    m_imagesInFlight[imgIndex] = m_inFlightFences[m_curretFrameIndex];

    // Reset fences into 'unsignaled' state
    vkResetFences(m_device, 1, &m_inFlightFences[m_curretFrameIndex]);
#endif

    submitCommandBuffer(m_curretFrameIndex, imgIndex);
    submitPresentation(m_curretFrameIndex, imgIndex);
}

// --------------------------<<  Exit  >>----------------------------
//
//  VkPhysicalDevice - automatically handled
//
// ------------------------------------------------------------------

void VulkanManager::cleanSwapChain()
{
    for (auto framebuffer : m_swapchainFrameBuffers)
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    vkFreeCommandBuffers(m_device, m_commandPool,   // free and reuse command buffers instead of creating a new one
                         static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
    vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    for (auto imageView : m_swapchainImageViews)
        vkDestroyImageView(m_device, imageView, nullptr);
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

void VulkanManager::cleanVulkan()
{
    // wait for any remaining asyncronous operations before cleanup
    vkDeviceWaitIdle(m_device);

    cleanSwapChain();

    // extensions must be destroyed before vulkan instance
    if (enableValidationLayers)
        destroyDebugUtilsMessengerEXT(m_VkInstance, &m_debugMessenger, nullptr);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_VkInstance, m_windowSurface, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);

    PRINTLN("Cleaned up Vulkan");
}
