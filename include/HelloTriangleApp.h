#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <vector>

class HelloTriangleApp
{
public:
    HelloTriangleApp();
    virtual ~HelloTriangleApp();

    void run();

private:
    void    initGLFW();
    void    mainLoop();

    void    initVulkan();
    void    createVulkanInstance();
    void    createDebugMessenger();
    bool    checkValidationLayerSupport();
    void    getRequiredVKExtensions(std::vector<const char*>&);

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

    void    cleanup();


    // GLFW Window
    GLFWwindow*     window;
    const uint32_t  WIDTH;
    const uint32_t  HEIGHT;

    // << Vulkan Instance >> connects between the application and the Vulkan library.
    VkInstance      m_VkInstance;

    // << Validation Layers >> custom error-checking method
    const std::vector<const char*>  m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
    VkDebugUtilsMessengerEXT        m_debugMessenger;
#if defined(NDEBUG)
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
    // Message callback handler
#endif
};