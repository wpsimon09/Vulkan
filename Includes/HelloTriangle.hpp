//
// Created by wpsimon09 on 25/07/24.
//

#ifndef HELLOTRIANGLE_HPP
#define HELLOTRIANGLE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <cstring>
#include "DebugInfoLog.hpp"
#include "Utils.hpp"
#include <set>
#include <glm/glm.hpp>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};


/*
#ifndef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers =true ;
#endif
*/

const bool enableValidationLayers =true ;


class HelloTriangle {
public:
    void run();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
        );

private:
    void InitWindow();

    void InitVulkan();
    void CreateInstance();
    bool CheckValidationLayerSupport();
    std::vector<const char *> GetRequiredExtentions();
    void SetUpDebugMessenger();
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void PickPhysicalDevice();
    void CreateSwapChain();
    void CreateLogicalDevice();
    void CreateSurface();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFrameBuffers();
    void CreateCommandPool();
    void CreateVertexBuffers();
    void CreateCommandBuffers();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void CreateSyncObjects();


    void CleanupSwapChain();
    void RecreateSwapChain();

    void MainLoop();
    void DrawFrame();

    void CleanUp();

    //----------
    // CALLBACKS
    //----------
    static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height);

    //-----------------
    // VULKAN STUFF
    //-----------------
    GLFWwindow* m_window;
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessanger;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device;

    VkQueue m_graphicsQueue;
    VkQueue m_presentationQueue;

    VkSurfaceKHR m_sruface;

    VkSwapchainKHR m_swapChain;

    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFrameBuffers;

    VkRenderPass m_renderPass;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;

    VkCommandPool m_comandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;

    //-----------------
    // OTHERS
    //-----------------
    uint32_t currentFrame = 0;
    bool m_frameBufferResized = false;
    ApplicationStatusNotifier m_appNotifier;
};

#endif //HELLOTRIANGLE_HPP
