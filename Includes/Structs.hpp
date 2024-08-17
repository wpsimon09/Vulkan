#include <vulkan/vulkan_core.h>

struct BufferCreateInfo {
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    VkDevice logicalDevice;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;
};
