#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

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

    void    cleanup();


    GLFWwindow*     window;
    const uint32_t  WIDTH;
    const uint32_t  HEIGHT;

    VkInstance      m_VkInstance;
};