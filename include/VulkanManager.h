#include <vulkan/vulkan.h>

#include <vector>
#include <string>


class VulkanManager
{
public:
    void    initVulkan();

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

    void    cleanVulkan();

    // << Vulkan Instance >> connects between the application and the Vulkan library.
    VkInstance                      m_VkInstance;
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