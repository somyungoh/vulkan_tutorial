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
    void    drawFrame();
    void    setFrameBufferResized(bool);

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

    bool    createWindowSurface();

    // << Device Extensions >>
    bool    checkDeviceExtensionSupport(VkPhysicalDevice);

    // << Swap Chain >>
    bool                    createSwapChain();
    bool                    recreateSwapChain();
    void                    cleanSwapChain();
    SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice);
    VkSurfaceFormatKHR      chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>&);
    VkPresentModeKHR        choosePresentMode(const std::vector<VkPresentModeKHR>&);
    VkExtent2D              chooseExtent2D(const VkSurfaceCapabilitiesKHR&);

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

    // << Rendering & Presentation >>
    bool  createSyncObjects();
    bool  acquireNextImageIndex(const uint32_t frameIndex, uint32_t &imageIndex);
    bool  submitCommandBuffer(const uint32_t frameIndex, const uint32_t imageIndex);
    bool  submitPresentation(const uint32_t frameIndex, const uint32_t imageIndex);

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
    GLFWwindow*                     m_window;
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

    // << Rendering & Presentation >>
    uint32_t                        m_curretFrameIndex;
    bool                            m_frameBufferResized;
    std::vector<VkSemaphore>        m_imageAvailableSemaphores;
    std::vector<VkSemaphore>        m_renderFinishedSemaphores;
    std::vector<VkFence>            m_inFlightFences;
    std::vector<VkFence>            m_imagesInFlight;
};
