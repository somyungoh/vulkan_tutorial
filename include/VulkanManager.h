#pragma once

#include <vulkan/vulkan.h>

#include <vector>

struct QueueFamilyIndices;
struct SwapchainSupportDetails;
struct GLFWwindow;

class VulkanManager
{
public:
    VulkanManager();

    void    initVulkan(GLFWwindow*);
    void    cleanVulkan();


private:
    bool                        createVulkanInstance();

    bool                        createDebugMessenger();
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

    bool        loadPhysicalDevice();
    u_int32_t   rateDeviceSuitability(VkPhysicalDevice);
    bool        isDeviceSuitable(VkPhysicalDevice);

    // << Queue Family >> subset of vulkan commands
    QueueFamilyIndices    findQueueFamilies(VkPhysicalDevice);

    bool    createLogicalDevice();

    bool    createWindowSurface(GLFWwindow*);

    // << Device Extensions >>
    bool    checkDeviceExtensionSupport(VkPhysicalDevice);

    // << Swap Chain >>
    bool                    createSwapChain(GLFWwindow*);
    SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice);
    VkSurfaceFormatKHR      chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
    VkPresentModeKHR        choosePresentMode(const std::vector<VkPresentModeKHR>&);
    VkExtent2D              chooseExtent2D(GLFWwindow*, const VkSurfaceCapabilitiesKHR&);

    // << Image Views >>
    bool createImageViews();

    // << Graphics Pipeline >>
    bool            createGraphicsPipeline();
    VkShaderModule  createShaderModule(const std::vector<char>&);

    // << Render Passes >>
    bool            createRenderPass();

private:
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

    // << Swap Chain >>
    VkSwapchainKHR                  m_swapchain;
    std::vector<VkImage>            m_swapchainImages;
    VkFormat                        m_swapchainImageFormat;
    VkExtent2D                      m_swapchainExtent;

    // << Image Views >>
    std::vector<VkImageView>        m_swapchainImageViews;

    // << Graphics Pipeline >>
    VkRenderPass                    m_renderPass;
    VkPipelineLayout                m_pipelineLayout;
};