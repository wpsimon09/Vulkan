#include <vulkan/vulkan_core.h>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


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
    glm::vec3 normal;
    glm::vec2 uv;



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

    static std::array<VkVertexInputAttributeDescription,4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription,4> attributeDescriptions;
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

        //which vertex array binding to use
        attributeDescriptions[2].binding = 0;
        //location in shader
        attributeDescriptions[2].location = 2;
        //vec2 has 2 32-bit float components
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        //offset to the position
        attributeDescriptions[2].offset = offsetof(Vertex, normal);

        //which vertex array binding to use
        attributeDescriptions[3].binding = 0;
        //location in shader
        attributeDescriptions[3].location = 3;
        //vec2 has 2 32-bit float components
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        //offset to the position
        attributeDescriptions[3].offset = offsetof(Vertex, uv);

        return attributeDescriptions;
    }

    bool operator == (const Vertex& other) const {
        return pos==other.pos && color == other.color && uv == other.uv && normal == other.normal;
    }

};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsAndComputeFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;

    bool isComplete() const { return graphicsAndComputeFamily.has_value() && presentFamily.has_value() && presentFamily.has_value();  }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct UniformBufferObject {
    alignas(16)glm::vec3 camPos;
    alignas(16)glm::vec3 lightPos;
    alignas(16)glm::mat4 model;
    alignas(16)glm::mat4 view;
    alignas(16)glm::mat4 projection;
    alignas(16)glm::mat4 normal;
};

struct ImageCreateInfo {
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    VkDevice logicalDevice;
    VkDeviceSize size;
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    uint32_t width;
    uint32_t height;
    uint32_t mipLevels = 1;
    VkImageUsageFlags usage;
    VkMemoryPropertyFlags memoryProperteis = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
};

struct ImageLayoutDependencyInfo {
    VkDevice logicalDevice;
    VkCommandBuffer commandBuffer;
    VkQueue transformQueue;
};

enum GEOMETRY_TYPE {
    PLANE = 0,
    CUBE = 1,
    SPHERE = 2,
    MODEL = 3,
};

struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            size_t h1 = hash<glm::vec3>()(vertex.pos);
            size_t h2 = hash<glm::vec3>()(vertex.normal);
            size_t h3 = hash<glm::vec2>()(vertex.uv);
            size_t h4 = hash<glm::vec3>()(vertex.color); // Hashing the color field

            return (((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1)) ^ (h4 << 1);
        }
    };
}


