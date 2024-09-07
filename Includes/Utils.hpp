//
// Created by wpsimon09 on 26/07/24.
//

#ifndef UTILS_HPP
#define UTILS_HPP
#include <optional>
#include <set>
#include <vulkan/vulkan.hpp>
#include <limits>
#include <algorithm>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

#include "Structs.hpp"


const std::vector<const char *> deviceExtentions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                             const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfor,
                                             const VkAllocationCallbacks *pAllocator,
                                             VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        return func(instance, pCreateInfor, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                          const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


inline QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

    int i = 0;
    for (auto &queueFamily: queueFamilyProperties) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            indices.transferFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }
        if (indices.isComplete()) break;

        i++;
    }

    return indices;
}

inline bool CheckDeviceExtentionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availablExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availablExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtentions.begin(), deviceExtentions.end());

    for (auto &availabl_extension: availablExtensions) {
        requiredExtensions.erase(availabl_extension.extensionName);
    }

    return requiredExtensions.empty();
}


inline SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount,
                                                  details.presentModes.data());
    }

    return details;
}

inline bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    std::cout << '\t' << deviceProperties.deviceName << std::endl;

    QueueFamilyIndices indices = FindQueueFamilies(device, surface);

    bool extensionsSupported = CheckDeviceExtentionSupport(device);

    bool swapChainAdequtate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);

        swapChainAdequtate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && indices.isComplete() &&
           swapChainAdequtate && supportedFeatures.samplerAnisotropy;
}

inline VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat: availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace ==
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

inline VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &availblePresentMode: availablePresentModes) {
        if (availblePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availblePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

inline VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        //we are going with the window size to be set as current extend
        return capabilities.currentExtent;
    }
    //teh current extend is max of uint32_t and we adjust max width and height for the swap chain images instead
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D acctualExtend = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    acctualExtend.width = std::clamp(acctualExtend.width, capabilities.minImageExtent.width,
                                     capabilities.maxImageExtent.width);
    acctualExtend.height = std::clamp(acctualExtend.height, capabilities.minImageExtent.height,
                                      capabilities.maxImageExtent.height);

    return acctualExtend;
}

static inline std::vector<char> readFile(const std::string &fileName) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        const auto err = "Failed to open file at path: " + fileName;
        throw std::runtime_error(err);
    }

    //create buffer to hold the binary
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    //go back to the begining and read the file again to get the content
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}


static inline VkShaderModule createShaderModuel(VkDevice device, std::vector<char> &shaderSPIRV) {
    VkShaderModuleCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderSPIRV.size();
    createInfo.pNext = nullptr;
    //cast from char to uint32_t
    createInfo.pCode = reinterpret_cast<const uint32_t *>(shaderSPIRV.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Could not create shader module");
    }

    shaderSPIRV.clear();

    return shaderModule;
}

static inline uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice) {
    //get the available types of memories on the GPU
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for(uint32_t i = 0; i<memProperties.memoryTypeCount; i++) {
        if(typeFilter & (1 << i) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Fialed to find suitable memmory type");
}

static inline void CreateBuffer(const BufferCreateInfo &bufferCreateInfo, VkBuffer& buffer, VkDeviceMemory &bufferMemory ){
    QueueFamilyIndices indices = FindQueueFamilies(bufferCreateInfo.physicalDevice, bufferCreateInfo.surface);
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferCreateInfo.size;
    // might be more using | operator
    bufferInfo.usage = bufferCreateInfo.usage;
    // specifies if it can be shared between queue families
    // we will use it only for the graphics family
    std::vector<uint32_t> sharedQueueFamilies = {indices.graphicsFamily.value(), indices.transferFamily.value()};

    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(sharedQueueFamilies.size());
    bufferInfo.pQueueFamilyIndices = sharedQueueFamilies.data();

    //sparse buffer memmory
    bufferInfo.flags = 0;

    if(vkCreateBuffer(bufferCreateInfo.logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vertex Buffer");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(bufferCreateInfo.logicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, bufferCreateInfo.properties, bufferCreateInfo.physicalDevice);

    if(vkAllocateMemory(bufferCreateInfo.logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate memory");
    }

    vkBindBufferMemory(bufferCreateInfo.logicalDevice, buffer,bufferMemory, 0);
}

static inline void CreateImage(const ImageCreateInfo &createImageInfo, VkImage &image, VkDeviceMemory &textureMemory) {

    QueueFamilyIndices indices = FindQueueFamilies(createImageInfo.physicalDevice, createImageInfo.surface);
    VkImageCreateInfo imageInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};

    //what coordinate systems will be used for the image access
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = createImageInfo.width;
    imageInfo.extent.height = createImageInfo.height;
    // 1 textel on Z axis not 0 texels
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    //use the same format as pixels that were loaded
    //using different format might result in crash during copying
    imageInfo.format = createImageInfo.format;
    imageInfo.tiling = createImageInfo.imageTiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = createImageInfo.usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    std::vector<uint32_t> sharedQueueFamilies = {indices.graphicsFamily.value(), indices.transferFamily.value()};
    imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(sharedQueueFamilies.size());
    imageInfo.pQueueFamilyIndices = sharedQueueFamilies.data();

    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if(vkCreateImage(createImageInfo.logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vulkan image ");
    }

    VkMemoryRequirements memReqirements;
    vkGetImageMemoryRequirements(createImageInfo.logicalDevice, image, &memReqirements);

    VkMemoryAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memReqirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memReqirements.memoryTypeBits, createImageInfo.memoryProperteis,createImageInfo.physicalDevice);

    if(vkAllocateMemory(createImageInfo.logicalDevice, &allocInfo, nullptr, &textureMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate image memory");
    }
    vkBindImageMemory(createImageInfo.logicalDevice, image, textureMemory, 0);
}

inline static VkCommandBuffer BeginSingleTimeCommand(VkDevice logicalDevice, VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

inline static void EndSingleTimeCommand(VkDevice logicalDevice,VkCommandPool commandPool,VkCommandBuffer commandBuffer, VkQueue queue) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

static inline void CopyBuffer(VkDevice logicalDevice,VkQueue transferQueue,VkCommandPool transferCommandPool,VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

    VkCommandBuffer commandBuffer = BeginSingleTimeCommand(logicalDevice, transferCommandPool);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer,1,&copyRegion);

    EndSingleTimeCommand(logicalDevice, transferCommandPool, commandBuffer, transferQueue);
}

static inline void CopyBuffer(VkDevice logicalDevice,VkCommandBuffer commandBuffer,VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

    //VkCommandBuffer commandBuffer = BeginSingleTimeCommand(logicalDevice, transferCommandPool);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer,1,&copyRegion);

    //EndSingleTimeCommand(logicalDevice, transferCommandPool, commandBuffer, transferQueue);
}

static inline void CopyBufferToImage(ImageLayoutDependencyInfo dependencyInfo, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    //VkCommandBuffer commandBuffer = BeginSingleTimeCommand(dependencyInfo.logicalDevice, dependencyInfo.commandPool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset={0,0,0};
    region.imageExtent = {width, height,1};

    vkCmdCopyBufferToImage(dependencyInfo.commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    //EndSingleTimeCommand(dependencyInfo.logicalDevice, dependencyInfo.commandPool, commandBuffer, dependencyInfo.transformQueue);
}

static inline void TransferImageLayout(ImageLayoutDependencyInfo dependencyInfo,VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {

    VkImageMemoryBarrier barrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }else if( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }else {
        throw std::invalid_argument("Unsupported layout transtiion");
    }

    vkCmdPipelineBarrier(
        dependencyInfo.commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}



static inline VkImageView GenerateImageView(VkDevice logicalDevice,VkImage image, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB) {
    VkImageViewCreateInfo viewInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imgView;
    if(vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imgView) != VK_SUCCESS) {
        throw std::runtime_error("Filed to create image views ");
    }

    return imgView;
}


static inline void GenerateSphere(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {
    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359f;
    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
    {
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            Vertex tempVertex {};
            tempVertex.pos = glm::vec3(xPos, yPos, zPos);
            tempVertex.normal = glm::vec3(xPos, yPos, zPos);
            tempVertex.uv = glm::vec2(xSegment, ySegment);

            vertices.push_back(tempVertex);
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else
        {
            for (int x = X_SEGMENTS; x >= 0; --x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }

}


#endif //UTILS_HPP
