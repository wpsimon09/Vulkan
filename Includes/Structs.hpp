#include <vulkan/vulkan_core.h>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES


struct BufferCreateInfo {
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    VkDevice logicalDevice;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;
};


enum APPLICATION_STATUS {
    IDLE = 0,
    RUNNING = 1,
    OFF = -1
};

struct ApplicationStatusNotifier {
    APPLICATION_STATUS appStatus = RUNNING;
    bool isDirty = true;

    void NotifyChange() {
        if (isDirty) {
            std::cout << "Application is: ";
            switch (appStatus) {
                case IDLE: {
                    std::cout << "Idle";
                    break;
                }
                case RUNNING: {
                    std::cout << "Running";
                    break;
                }
                case OFF: {
                    std::cout << "Off";
                    break;
                }
            }
            std::cout << std::endl;
            isDirty = false;
        }
    }
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        //we will only use one binding
        //ths will be its index
        bindingDescription.binding = 0;
        //nubmer of bites between each data entry
        bindingDescription.stride = sizeof(Vertex);
        //move to the next vertex after each vertex not aftera each instance
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription,2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription,2> attributeDescriptions;
        //which vertex array binding to use
        attributeDescriptions[0].binding = 0;
        //location in shader
        attributeDescriptions[0].location = 0;
        //vec2 has 2 32-bit float components
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        //offset to the position
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        //which vertex array binding to use
        attributeDescriptions[1].binding = 0;
        //location in shader
        attributeDescriptions[1].location = 1;
        //vec2 has 2 32-bit float components
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        //offset to the position
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;

    bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value() && presentFamily.has_value();  }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct UniformBufferObject {
    alignas(16)glm::vec3 camPos;
    alignas(16)glm::mat4 model;
    alignas(16)glm::mat4 view;
    alignas(16)glm::mat4 projection;
    alignas(16)glm::mat4 normal;
};

