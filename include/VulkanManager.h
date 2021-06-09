#pragma once

#include <vulkan/vulkan.h>

#include <vector>

struct QueueFamilyIndices;
struct GLFWwindow;

class VulkanManager
{
public:
    VulkanManager();

    void    initVulkan(GLFWwindow*);

private:
    void                        createVulkanInstance();

    void                        createDebugMessenger();
    bool                        checkValidationLayerSupport();
    std::vector<const char*>    loadVKExtensions();

    // Message callback function
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT,
        VkDebugUtilsMessageTypeFlagsEXT,
        const VkDebugUtilsMessengerCallbackDataEXT*,
        void*
    );
    VkResult createDebugUtilsMessengerEXT(
        VkInstance,
        const VkDebugUtilsMessengerCreateInfoEXT*,
        const VkAllocationCallbacks*,
        VkDebugUtilsMessengerEXT*
    );
    VkResult destroyDebugUtilsMessengerEXT(
        VkInstance,
        const VkDebugUtilsMessengerEXT*,
        const VkAllocationCallbacks*
    );

    void        loadPhysicalDevice();
    u_int32_t   rateDeviceSuitability(VkPhysicalDevice);
    bool        isDeviceSuitable(VkPhysicalDevice);

    // << Queue Family >> subset of vulkan commands
    QueueFamilyIndices    findQueueFamilies(VkPhysicalDevice);

    void    createLogicalDevice();

    void    createWindowSurface(GLFWwindow*);

    // << Device Extensions >>
    bool    checkDeviceExtensionSupport(VkPhysicalDevice);

    void    cleanVulkan();



    // << Vulkan Instance >> connects between the application and the Vulkan library.
    VkInstance                      m_VkInstance;
    // << Validation Layers >> custom error-checking method
    const std::vector<const char*>  m_validationLayers;
    VkDebugUtilsMessengerEXT        m_debugMessenger;
#if defined(NDEBUG)
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    // << Physical Device >>
    VkPhysicalDevice                m_physicalDevice;

    // << Logical Device >>
    VkDevice                        m_device;
    VkQueue                         m_graphicsQueue;
    VkQueue                         m_presentationQueue;

    // << Window Surface >>
    VkSurfaceKHR                    m_windowSurface;

    // << Device Extensions >>
    const std::vector<const char*>  m_deviceExtensions;
};