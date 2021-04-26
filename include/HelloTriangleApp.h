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
    bool    checkValidationLayerSupport();

    void    cleanup();


    GLFWwindow*     window;
    const uint32_t  WIDTH;
    const uint32_t  HEIGHT;

    // << Vulkan Instance >> connects between the application and the Vulkan library.
    VkInstance      m_VkInstance;

    // << Validation Layers >> custom error-checking method
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
#if defined(NDEBUG)
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};