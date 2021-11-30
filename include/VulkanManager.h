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
    bool createRenderPass();

    // << Frame Buffers >>
    bool createFrameBuffers();

    // << Command Buffers >>
    bool createCommandPool();
    bool createCommandBuffers();

    // << Drawing >>
    bool    createSemaphores();
    uint32  acquireNextImageIndex();
    bool    submitCommandBuffer(const uint32);

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

    // << Render Pass >>
    VkRenderPass                    m_renderPass;

    // << Graphics Pipeline >>
    VkPipelineLayout                m_pipelineLayout;
    VkPipeline                      m_graphicsPipeline;

    // << Frame Buffers >>
    std::vector<VkFramebuffer>      m_swapchainFrameBuffers;

    // << Command Buffers >>
    VkCommandPool                   m_commandPool;
    std::vector<VkCommandBuffer>    m_commandBuffers;

    // << Semaphores >>
    VkSemaphore                     m_imageAvailableSemaphore;
    VkSemaphore                     m_renderFinishedSemaphore;
};
