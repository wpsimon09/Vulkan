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
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include "Camera/Camera.hpp"
#include "memory"
#include "VertexData.hpp"
#include <stb/stb_image.h>
#include <unordered_map>
#include <sys/prctl.h>

#include "Material/Material.hpp"

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::string MODEL_PATH = "Includes/Models/TIE Fighter.obj";
const std::string TEXTURE_PATH = "Textures/TIE_color.png";

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


class VulkanApp {
public:
    void run();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
        );

private:
    //-------------------------
    // INITIAL VULKAN SET UP
    //-------------------------
    void CreateCamera();
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
    VkSampleCountFlagBits GetMaxUsableSampleCount();
    //------------------------------

    //-----------------------------
    // PIPELINE AND PRESENTATION
    //-----------------------------
    void CreateImageViews();
    void CreateRenderPass();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    //------------------------------

    //-----------------------------------------------
    // BUFFERS, (VERTEX, COMMAND ETC.) + DESCRIPTORS
    //-----------------------------------------------
    void CreateFrameBuffers();
    void CreateCommandPool();
    void CreateVertexBuffers();
    void CreateIndexBuffers();
    void CreateUniformBuffers();
    void CreateCommandBuffers();
    void CreateDepthResources();
    void CreateShaderStorageBuffer();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    VkCommandBuffer StartRecordingCommandBuffer();
    void FlushCommandBuffer(VkCommandBuffer commandBuffer);
    void CreateDescriptorPool();
    void CreateDescriptorSet();
    //-----------------------------------

    //---------------------
    // TEXTURES AND IMAGES
    //---------------------
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateColorResources();
    //---------------------

    //---------------------
    // SYNCHRONIZATION
    //---------------------
    void CreateSyncObjects();
    void UpdateUniformBuffer(uint32_t currentImage);
    //---------------------


    //-------------------------
    // MAIN LOOP AND DRAWING
    //-------------------------
    void MainLoop();
    void DrawFrame();
    //-------------------------

    //-------------
    // CLEAN UP
    //--------------
    void CleanupSwapChain();
    void RecreateSwapChain();
    void CleanUp();
    //---------------

    //--------------------
    // CALLBACKS
    //--------------------
    static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height);
    static void MousePositionCallback(GLFWwindow *window, double xpos, double ypos);
    static void MouseClickCallback(GLFWwindow *window, int button, int action, int mods);
    static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    void ProcessKeyboardInput();
    //---------------------

    //---------------------
    // MISCELLANEOUS
    //---------------------
    void GenerateGeometryVertices(GEOMETRY_TYPE geometryType);
    void LoadModel();
    VkFormat FindDepthFormat();
    //-----------------


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
    VkQueue m_transferQueue;
    VkQueue m_computeQueue;

    VkSurfaceKHR m_sruface;
    VkSwapchainKHR m_swapChain;

    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFrameBuffers;

    VkRenderPass m_renderPass;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;

    VkCommandPool m_comandPool;
    VkCommandPool m_transferCommandPool;
    VkCommandBuffer m_transferCommandBuffer;
    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;

    std::vector<VkBuffer> m_shaderStorageBuffer;
    std::vector<VkDeviceMemory> m_shaderStorageBufferMemory;

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;

    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

    VkImage m_textureImage;
    VkImageView m_textureImageView;
    VkDeviceMemory m_textureImageMemory;
    VkSampler m_textureSampler;

    VkImage m_colorImage;
    VkDeviceMemory m_colorImageMemory;
    VkImageView m_colorImageView;

    VkImage m_depthImage;
    VkDeviceMemory m_depthMemory;
    VkImageView m_depthImageView;

    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;

    VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;


    //-----------------
    // OTHERS
    //-----------------
    uint32_t currentFrame = 0;
    bool m_frameBufferResized = false;
    ApplicationStatusNotifier m_appNotifier;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Material> m_material;
    double m_lastX;
    double m_lastY;
    bool m_isMousePressed = false;
    bool m_isFirstMouse = true;
    GEOMETRY_TYPE m_geometryType;

    glm::vec3 m_lightPos = glm::vec3(0.0f);

    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;



};

#endif //HELLOTRIANGLE_HPP
