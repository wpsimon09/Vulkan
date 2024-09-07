//
// Created by wpsimon09 on 25/07/24.
//

#include "VulkanApp.hpp"

#include <chrono>
#include <emmintrin.h>
#include <random>
#include <thread>
#include <unistd.h>
#include <unordered_map>

#include "tiny_obj_loader/tiny_obj_loader.h"


void VulkanApp::run() {
    InitWindow();

    InitVulkan();
    MainLoop();

    CleanUp();
}

VkBool32 VulkanApp::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                  void *pUserData) {
    if (messageSeverity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        GetSeverity(messageSeverity);
        GetMessageType(messageType);
        std::cerr <<
                "\n File:\t" << __FILE__ <<
                "\n Message:\t" << pCallbackData->pMessage <<
                "\n Message ID:\t" << pCallbackData->messageIdNumber <<
                "\n Object name:\t" << pCallbackData->pObjects->pObjectName <<
                std::endl;
    }
    return VK_FALSE;
}

void VulkanApp::InitWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,GLFW_TRUE);
    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    CreateCamera();

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FrameBufferResizeCallback);
    glfwSetCursorPosCallback(m_window, MousePositionCallback);
    glfwSetMouseButtonCallback(m_window, MouseClickCallback);
    glfwSetScrollCallback(m_window, MouseScrollCallback);
}

void VulkanApp::InitVulkan() {
    CreateInstance();
    SetUpDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateColorResources();
    CreateDepthResources();
    CreateRenderPass();
    GenerateGeometryVertices(MODEL);
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateComputePipeline();
    CreateFrameBuffers();
    CreateCommandPool();

    auto start = std::chrono::high_resolution_clock::now();

    CreateTextureImage();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken while generating material resources: " << duration.count() << " ms" << std::endl;

    CreateTextureImageView();
    CreateTextureSampler();
    CreateVertexBuffers();
    CreateShaderStorageBuffer();
    CreateIndexBuffers();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSet();
    CreateCommandBuffers();
    CreateSyncObjects();
}

void VulkanApp::CreateCamera() {
    this->m_camera = std::make_unique<Camera>(m_window);
}

void VulkanApp::CreateInstance() {
    if (enableValidationLayers && !this->CheckValidationLayerSupport()) {
        throw std::runtime_error("Requested validation layers were not found");
    } else {
        std::cout << "Valiation layers found\n";
    }

    //--------
    //APP INFO
    //--------
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan app";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    appInfo.pNext = nullptr;

    prctl(PR_SET_NAME, "MyVulkanApp", 0, 0, 0);

    //--------------
    // INSTANCE INFO
    //--------------
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    //----------
    //EXTENSIONS
    //----------
    auto extentions = GetRequiredExtentions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extentions.size());
    createInfo.ppEnabledExtensionNames = extentions.data();

    //----------------
    // CREATE INSTANCE
    //----------------
    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create intance \n");
    } else {
        std::cout << "Vulkan instance created successfuly \n";
    }
}

bool VulkanApp::CheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName: validationLayers) {
        bool layerFound = false;
        for (const auto &layerProperties: availableLayers) {
            std::cout << '\t' << layerProperties.layerName << '\n';
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)
            return false;
    }
    return true;
}

void VulkanApp::MainLoop() {
    while (!glfwWindowShouldClose(m_window)) {
        ProcessKeyboardInput();
        DrawFrame();
        m_appNotifier.NotifyChange();
        glfwPollEvents();
    }
    vkDeviceWaitIdle(m_device);
}

void VulkanApp::DrawFrame() {
    // wait for previous frame to finish drawind
    vkWaitForFences(m_device, 1, &m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    //get image from swap chain to draw into
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[currentFrame],
                                            VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain iamge");
    }

    //reset fences after we are sure that we can continue rendering
    vkResetFences(m_device, 1, &m_inFlightFences[currentFrame]);

    UpdateUniformBuffer(currentFrame);

    //clear the command buffer so that it can record new information
    //here is acctual draw command and pipeline binding, scissors and viewport configuratio
    vkResetCommandBuffer(m_commandBuffers[currentFrame], 0);
    RecordCommandBuffer(m_commandBuffers[currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore syncSemaphors[] = {m_imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};


    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = syncSemaphors;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit drawing command buffer");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(m_presentationQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_frameBufferResized) {
        RecreateSwapChain();
        m_frameBufferResized = false;
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image ");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApp::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
}

void VulkanApp::PickPhysicalDevice() {
    //-----------------------------------
    // GET ALL AVAILABLE PHYSICAL DEVICES
    //-----------------------------------
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    std::cout << "Found " << deviceCount << " physical devices: \n";
    if (deviceCount == 0) {
        throw std::runtime_error("There are no physical devices on the machine ");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    //----------------------------------
    // PICK THE PHYSICAL DEVICE SUITABLE
    //----------------------------------
    for (auto &device: devices) {
        if (isDeviceSuitable(device, m_sruface)) {
            m_physicalDevice = device;
            VkPhysicalDeviceProperties props;
            m_msaaSamples = GetMaxUsableSampleCount();
            vkGetPhysicalDeviceProperties(device, &props);
            std::cout << "Using:" << props.deviceName << std::endl;
            break;
        }
    }
    if (m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to found any suitable GPU");
    }
}

void VulkanApp::CreateSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice, m_sruface);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, m_window);

    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;

    //images in swap chain (array of images)
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    //make sure to not go over the limit of images in swap chain
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_sruface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    //layer for setreoscopic 3D application
    createInfo.imageArrayLayers = 1;
    //how images will be used
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice, m_sruface);
    uint32_t queueFamilyIndecies[] = {indices.graphicsAndComputeFamily.value(), indices.presentFamily.value()};

    //if presentation and graphics queue family are the same use the exlusive mode
    //otherwise use concurent mode
    if (indices.graphicsAndComputeFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndecies;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    //horizontal flip, 90 deg rotation
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    //blend with other windows ? no
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    //mailbox, fifo etc...
    createInfo.presentMode = presentMode;
    //if other window is infrotn of current window dont calculate pixels
    createInfo.clipped = VK_TRUE;
    //it can happend that swap chain will have to be recreted again, we wont do it here hence NULL
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain");
    }
    std::cout << "Swapchain created !\n";

    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

    if (m_swapChainImages.empty())
        throw std::runtime_error("No swap chain images retrieved");
    std::cout << "Retrieved " << m_swapChainImages.size() << " swap chain images\n";
}

void VulkanApp::CreateImageViews() {
    m_swapChainImageViews.resize(m_swapChainImages.size());
    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        m_swapChainImageViews[i] = GenerateImageView(m_device, m_swapChainImages[i], 1, m_swapChainImageFormat);
    }
}

void VulkanApp::CreateRenderPass() {
    //-----------------------
    // COLOUR ATTACHEMNT INFO
    //-----------------------
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapChainImageFormat;
    colorAttachment.samples = m_msaaSamples;
    //before render call colou and depth
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //after render call colour and depth
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //before render call
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //after render call
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;;

    //-----------------------
    // DEPTH ATTACHEMNT INFO
    //-----------------------
    VkAttachmentDescription depthAttachment{};
    //format should be the same as a depht image
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = m_msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //we dont want to read from the depth attachemnt once it was drawn into
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


    //------------------------
    // RESOLVE ATTACHEMTN INFO
    //------------------------
    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = m_swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


    //----------------------
    // ATTACHEMNT REFERENCES
    //----------------------
    VkAttachmentReference colorAttachmentRef{};
    //reference to the imaginary array of VkAttachmentDescription (we only have one)
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //---------
    // SUB PASS
    //---------
    VkSubpassDescription subPass{};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //reference to the colour attachemnt
    subPass.colorAttachmentCount = 1;
    subPass.pColorAttachments = &colorAttachmentRef;
    subPass.pDepthStencilAttachment = &depthAttachmentRef;
    subPass.pResolveAttachments = &colorAttachmentResolveRef;

    std::array<VkAttachmentDescription, 3> attachemnts = {colorAttachment, depthAttachment, colorAttachmentResolve};

    //--------------------
    // SUB PASS DEPENDENCY
    //--------------------
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    //subpass we are going to use since we only have one we will pass 0
    //dst subpass must be heigher than srcSubpass, only exception is if src is VK_SUBPASS_EXTERNAL
    dependency.dstSubpass = 0;
    //dependecy start
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    //we are not targeting any memmory so 0
    dependency.srcAccessMask = 0;

    //dependency end
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    //we want to access colour attachemnt so that we can write to it
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachemnts.size());
    renderPassInfo.pAttachments = attachemnts.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subPass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;


    if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass");
    }
}

std::vector<VkDescriptorSetLayoutBinding> VulkanApp::CreateComputeDescriptorSetLayout(int stratsFrom) {

    // UBO for delat time, SSBO for reads and SSBO for writes (3 bindings in total)
    std::vector<VkDescriptorSetLayoutBinding> particleDescriptorLayoutBindings(3);
    particleDescriptorLayoutBindings[0].binding = stratsFrom;
    particleDescriptorLayoutBindings[0].descriptorCount = 1;
    particleDescriptorLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    particleDescriptorLayoutBindings[0].pImmutableSamplers = nullptr;
    particleDescriptorLayoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    //Read SSBO
    particleDescriptorLayoutBindings[1].binding = stratsFrom+1;
    particleDescriptorLayoutBindings[1].descriptorCount = 1;
    particleDescriptorLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    particleDescriptorLayoutBindings[1].pImmutableSamplers = nullptr;
    particleDescriptorLayoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    //Write SSBO
    particleDescriptorLayoutBindings[2].binding = stratsFrom +2;
    particleDescriptorLayoutBindings[2].descriptorCount = 1;
    particleDescriptorLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    particleDescriptorLayoutBindings[2].pImmutableSamplers = nullptr;
    particleDescriptorLayoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;


}

void VulkanApp::CreateDescriptorSetLayout() {
    //FOR MVP
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    auto bindings = m_material->GetLayoutBindings(1);
    bindings.emplace_back(uboLayoutBinding);

    VkDescriptorSetLayoutCreateInfo layoutInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    layoutInfo.pNext = nullptr;

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    };


    auto computeBindings = CreateComputeDescriptorSetLayout(0);
    layoutInfo.bindingCount = static_cast<uint32_t>(computeBindings.size());
    layoutInfo.pBindings = computeBindings.data();
    layoutInfo.pNext = nullptr;

    if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_computeDescryptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute descriptor set layout");
    };
}

void VulkanApp::CreateDescriptorPool() {
    std::array<VkDescriptorPoolSize, 4> poolSizes{};

    // for UBO MVP
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    //for delta time UBO
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    // for Sampler
    poolSizes[2] = m_material->GetDescriptorPoolSize(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

    //for each frame in flight both read and write SSBO will be used, thus * 2
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) *2;

    VkDescriptorPoolCreateInfo poolInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool");
    }
}

void VulkanApp::CreateDescriptorSet() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();
    m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

    if (vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptors sets");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        //--------
        // UBO
        //--------
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;

        //----------
        // TEXTURE
        //----------
        /*VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_textureImageView;
        imageInfo.sampler = m_textureSampler;*/


        //--------
        // UBO
        //--------
        VkWriteDescriptorSet bufferDescriptorWrite;
        bufferDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        bufferDescriptorWrite.dstSet = m_descriptorSets[i];
        bufferDescriptorWrite.dstBinding = 0;
        bufferDescriptorWrite.dstArrayElement = 0;
        bufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bufferDescriptorWrite.descriptorCount = 1;
        bufferDescriptorWrite.pBufferInfo = &bufferInfo;
        bufferDescriptorWrite.pImageInfo = nullptr;
        bufferDescriptorWrite.pTexelBufferView = nullptr;
        bufferDescriptorWrite.pNext = nullptr;

        //----------
        // TEXTURE
        //----------
        /*descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;
        descriptorWrites[1].pBufferInfo = nullptr;
        descriptorWrites[1].pTexelBufferView = nullptr;*/

        auto descriptorWrites = m_material->GetDescriptorWrites(m_descriptorSets[i]);
        descriptorWrites.insert(descriptorWrites.begin(), bufferDescriptorWrite);

        vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                               nullptr);
    }
}

void VulkanApp::CreateGraphicsPipeline() {
    //----------------
    // SHADER CREATION
    //----------------
    auto vertShaderCode = readFile("Shaders/Compiled/TriangleVertex.spv");
    auto fragmentShaderCode = readFile("Shaders/Compiled/TriangleFragment.spv");
    std::cout << "Shader read sucessfuly\n";

    VkShaderModule vertexShaderModule = createShaderModuel(m_device, vertShaderCode);
    VkShaderModule fragmentShaderModule = createShaderModuel(m_device, fragmentShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertexShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pNext = nullptr;

    //allows to fill in constant variables in shaders using memory offset
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
    fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageInfo.module = fragmentShaderModule;
    fragmentShaderStageInfo.pName = "main";
    fragmentShaderStageInfo.pNext = nullptr;
    //allows to fill in constant variables in shaders using memory offset
    fragmentShaderStageInfo.pSpecializationInfo = nullptr;

    //todo: later add compute shader here!!
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragmentShaderStageInfo};

    //----------------
    // DINAMIC PIPLINE
    //----------------
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    //------------------
    // VERTEX ATTRIBUTES
    //------------------
    auto bindingDescription = Particle::getBindingDescription();
    auto attributeDescriptions = Particle::getAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    //-------------------
    // PRIMITIVE TOPOLOGY
    //-------------------
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = m_geometryType == SPHERE
                                           ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
                                           : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    //----------
    // VIEW PORT
    //----------
    VkViewport viewPort{};
    viewPort.x = 0.0f;
    viewPort.y = 0.0f;
    viewPort.width = (float) m_swapChainExtent.width;
    viewPort.height = (float) m_swapChainExtent.height;
    viewPort.minDepth = 0.0f;
    viewPort.maxDepth = 1.0f;

    //---------
    // SCISSORS
    //---------
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChainExtent;

    //---------------------------------------------------
    // SCISSORS AND VIEW PORT DYNAMIC ADD TO THE PIPELINE
    //---------------------------------------------------
    std::vector<VkDynamicState> dynamicStates2 = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynmicStateVPandScissors{};
    dynmicStateVPandScissors.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynmicStateVPandScissors.dynamicStateCount = static_cast<uint32_t>(dynamicStates2.size());
    dynmicStateVPandScissors.pDynamicStates = dynamicStates2.data();

    VkPipelineViewportStateCreateInfo viewPortState{};
    viewPortState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewPortState.viewportCount = 1;
    viewPortState.scissorCount = 1;

    //-----------
    // RASTERIZER
    //-----------
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode = m_geometryType == PLANE ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizerCreateInfo.depthBiasConstantFactor = 0.0F;
    rasterizerCreateInfo.depthBiasClamp = 0.0f;
    rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

    //--------------
    // MULTISAMPLING
    //--------------
    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{};
    multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleCreateInfo.rasterizationSamples = m_msaaSamples;
    multisampleCreateInfo.minSampleShading = 1.0f;
    multisampleCreateInfo.pSampleMask = nullptr;
    multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

    //-----------------------
    // DEPTH AND STENCIL TEST
    //-----------------------
    VkPipelineDepthStencilStateCreateInfo depthStencil{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
    };
    depthStencil.depthTestEnable = VK_TRUE;
    //should fragments that pass the depht test be written to the frame buffer ?
    depthStencil.depthWriteEnable = VK_TRUE;
    //fragemnts pass the depht test if their value is smaller than value allredy written in depth buffer
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    //allows us to keep only fragments that fall within specific range
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;

    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};


    //----------------
    // COLOUR BLENDING
    //----------------
    VkPipelineColorBlendAttachmentState colourBlendAttachmentCreateInfo{};
    colourBlendAttachmentCreateInfo.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colourBlendAttachmentCreateInfo.blendEnable = VK_FALSE;
    colourBlendAttachmentCreateInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colourBlendAttachmentCreateInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colourBlendAttachmentCreateInfo.colorBlendOp = VK_BLEND_OP_ADD;
    colourBlendAttachmentCreateInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colourBlendAttachmentCreateInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colourBlendAttachmentCreateInfo.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colourBlendCreateInfo{};
    colourBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colourBlendCreateInfo.logicOpEnable = VK_FALSE;
    colourBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colourBlendCreateInfo.attachmentCount = 1;
    colourBlendCreateInfo.pAttachments = &colourBlendAttachmentCreateInfo;
    colourBlendCreateInfo.blendConstants[0] = 0.0f;
    colourBlendCreateInfo.blendConstants[1] = 0.0f;
    colourBlendCreateInfo.blendConstants[2] = 0.0f;
    colourBlendCreateInfo.blendConstants[3] = 0.0f;


    //----------------
    // PIPELINE LAYOUT
    //----------------
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout !");
    }

    //------------------
    // PIPELINE CREATION
    //------------------
    VkGraphicsPipelineCreateInfo pipelineInfo{};

    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    //shaders
    pipelineInfo.pStages = shaderStages;
    //fixed functionality
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineInfo.pViewportState = &viewPortState;
    pipelineInfo.pRasterizationState = &rasterizerCreateInfo;
    pipelineInfo.pMultisampleState = &multisampleCreateInfo;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colourBlendCreateInfo;
    pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
    //pipeline layout for uniforms
    pipelineInfo.layout = m_pipelineLayout;
    //render pass
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    //pipeline handle
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline)) {
        throw std::runtime_error("Failed to create graphics pipeline");
    } else {
        std::cout << "Graphics pipeline created sucessfully !\n";
    }


    vkDestroyShaderModule(m_device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(m_device, fragmentShaderModule, nullptr);
}

void VulkanApp::CreateComputePipeline() {
    auto computeShaderCode = readFile("Shaders/Compiled/ParticlesCompute.spv");
    VkShaderModule computeShaderModule = createShaderModuel(m_device, computeShaderCode);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";
    computeShaderStageInfo.pName = nullptr;
    computeShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineLayoutCreateInfo computePipelineLayout{.sType =  VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    computePipelineLayout.pSetLayouts = &m_computeDescryptorSetLayout;
    computePipelineLayout.setLayoutCount  =1 ;
    if(vkCreatePipelineLayout(m_device, &computePipelineLayout, nullptr, &m_computePipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout !");
    }

    VkComputePipelineCreateInfo computePipelineInfo{.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    computePipelineInfo.layout = m_computePipelineLayout;
    computePipelineInfo.stage =

}

void VulkanApp::CreateFrameBuffers() {
    m_swapChainFrameBuffers.resize(m_swapChainImageViews.size());

    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        std::array<VkImageView, 3> attachments = {
            m_colorImageView,
            m_depthImageView,

            m_swapChainImageViews[i]
        };
        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = m_renderPass;
        frameBufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        frameBufferInfo.pAttachments = attachments.data();
        frameBufferInfo.width = m_swapChainExtent.width;
        frameBufferInfo.height = m_swapChainExtent.height;
        frameBufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &frameBufferInfo, nullptr, &m_swapChainFrameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create frame buffers from swap chain images");
        }
    }
}


void VulkanApp::CreateTextureImage() {
    TEXTURE_TYPE texturesToProcess[] = {TEXTURE_TYPE_ALBEDO, TEXTURE_TYPE_ARM, TEXTURE_TYPE_NORMAL};
    std::vector<std::string> paths = {
        "Textures/tie_albeo.png", "Textures/tie_arm.png", "Textures/normal.png"
    };

    VkBuffer stagingImageBuffer;
    VkDeviceMemory stagingImageMemory;

    BufferCreateInfo bufferInfo;
    bufferInfo.physicalDevice = m_physicalDevice;
    bufferInfo.logicalDevice = m_device;
    bufferInfo.surface = m_sruface;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.physicalDevice = m_physicalDevice;
    imageCreateInfo.logicalDevice = m_device;
    imageCreateInfo.surface = m_sruface;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageCreateInfo.imageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageCreateInfo.memoryProperteis = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;


    for (int i = 0; i < paths.size(); i++) {
        int texWidth, texHeight, texChanels;
        stbi_uc *pixels = stbi_load(paths[i].c_str(), &texWidth, &texHeight, &texChanels, STBI_rgb_alpha);

        std::cout << "Loading texture: " << paths[i] << std::endl;

        if (!pixels) {
            throw std::runtime_error("Failed to load textue");
        }

        m_material->GetTextures()[texturesToProcess[i]].maxMipLevels = static_cast<uint32_t>(std::floor(
                                                                           std::log2(std::max(texWidth, texHeight)))) +
                                                                       1;;
        imageCreateInfo.mipLevels = m_material->GetTextures()[texturesToProcess[i]].maxMipLevels;

        // times 4 becaus   e we have RGBA
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        bufferInfo.size = imageSize;
        CreateBuffer(bufferInfo, stagingImageBuffer, stagingImageMemory);

        void *data;
        vkMapMemory(m_device, stagingImageMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_device, stagingImageMemory);

        stbi_image_free(pixels);

        imageCreateInfo.width = texWidth;
        imageCreateInfo.height = texHeight;
        imageCreateInfo.size = imageSize;

        ImageLayoutDependencyInfo dependencyInfo{};
        dependencyInfo.commandBuffer = StartRecordingCommandBuffer();
        dependencyInfo.logicalDevice = m_device;
        dependencyInfo.transformQueue = m_transferQueue;

        CreateImage(imageCreateInfo, m_material->GetTextures()[texturesToProcess[i]].image, m_material->GetTextures()[texturesToProcess[i]].memory);

        TransferImageLayout(dependencyInfo, m_material->GetTextures()[texturesToProcess[i]].image, imageCreateInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,m_material->GetTextures()[texturesToProcess[i]].maxMipLevels);

        CopyBufferToImage(dependencyInfo,stagingImageBuffer, m_material->GetTextures()[texturesToProcess[i]].image, static_cast<uint32_t>(texWidth),static_cast<uint32_t>(texHeight));

        GenerateMipMaps(m_physicalDevice,dependencyInfo, m_material->GetTextures()[texturesToProcess[i]].image, texWidth, texHeight,m_material->GetTextures()[texturesToProcess[i]].maxMipLevels);

        FlushCommandBuffer(dependencyInfo.commandBuffer);
    }
    vkFreeMemory(m_device, stagingImageMemory, nullptr);
    vkDestroyBuffer(m_device, stagingImageBuffer, nullptr);
}

void VulkanApp::CreateCommandPool() {
    //retrieve all queue families from the GPU
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice, m_sruface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //use the graphics family for the drawing
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily.value();

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_comandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool !");
    }

    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();
    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_transferCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create transfer command pool");
    }
}

void VulkanApp::CreateVertexBuffers() {
    //-------------
    // BUFFER INFO
    //-------------
    BufferCreateInfo bufferInfo{};
    bufferInfo.size = sizeof(vertices[0]) * vertices.size();
    bufferInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    bufferInfo.surface = m_sruface;
    bufferInfo.logicalDevice = m_device;
    bufferInfo.physicalDevice = m_physicalDevice;

    //----------------
    // STAGING BUFFER
    //----------------
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    CreateBuffer(bufferInfo, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferInfo.size);
    vkUnmapMemory(m_device, stagingBufferMemory);

    //----------------
    // VERTEX BUFFER
    //----------------
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    CreateBuffer(bufferInfo, m_vertexBuffer, m_vertexBufferMemory);

    //-----------------------------------
    // MOVE THE MEMORY FROM STAGING
    // BUFFER TO ACCTUAL VERTEX BUFFER
    //----------------------------------
    CopyBuffer(m_device, m_transferQueue, m_transferCommandPool, stagingBuffer, m_vertexBuffer, bufferInfo.size);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

void VulkanApp::CreateIndexBuffers() {
    BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.physicalDevice = m_physicalDevice;
    bufferCreateInfo.logicalDevice = m_device;
    bufferCreateInfo.surface = m_sruface;

    bufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.size = sizeof(uint32_t) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    CreateBuffer(bufferCreateInfo, stagingBuffer, stagingMemory);

    //put data to the staging buffer
    void *data;
    vkMapMemory(m_device, stagingMemory, 0, bufferCreateInfo.size, 0, &data);
    memcpy(data, indices.data(), (size_t) bufferCreateInfo.size);
    vkUnmapMemory(m_device, stagingMemory);

    //create index buffer
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    CreateBuffer(bufferCreateInfo, m_indexBuffer, m_indexBufferMemory);

    //copy staging buffer to the index buffer
    CopyBuffer(m_device, m_transferQueue, m_transferCommandPool, stagingBuffer, m_indexBuffer, bufferCreateInfo.size);

    //clean up
    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingMemory, nullptr);
}

void VulkanApp::CreateUniformBuffers() {
    BufferCreateInfo bufferInfo{};
    bufferInfo.logicalDevice = m_device;
    bufferInfo.physicalDevice = m_physicalDevice;
    bufferInfo.surface = m_sruface;
    bufferInfo.size = sizeof(UniformBufferObject);
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    m_deltaTimeUBOBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    m_deltaTimeUBOMemory.resize(MAX_FRAMES_IN_FLIGHT);
    m_deltaTimeUBOBuffer.resize(MAX_FRAMES_IN_FLIGHT);


    // for MVP UBO
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CreateBuffer(bufferInfo, m_uniformBuffers[i], m_uniformBuffersMemory[i]);
        vkMapMemory(m_device, m_uniformBuffersMemory[i], 0, bufferInfo.size, 0, &m_uniformBuffersMapped[i]);
    }

    bufferInfo.size = sizeof(UBODeltaTime);
    //for delta time UBO
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        CreateBuffer(bufferInfo, m_deltaTimeUBOBuffer[i], m_deltaTimeUBOMemory[i]);
        vkMapMemory(m_device, m_deltaTimeUBOMemory[i], 0, bufferInfo.size, 0, &m_deltaTimeBufferMapped[i]);
    }

}

void VulkanApp::CreateTextureImageView() {
    //m_textureImageView = GenerateImageView(m_device, m_textureImage);

    for (auto &materialTexture: m_material->GetTextures()) {
        materialTexture.second.imageView = GenerateImageView(m_device, materialTexture.second.image,
                                                             materialTexture.second.maxMipLevels);
    }
}

void VulkanApp::CreateTextureSampler() {
    VkSamplerCreateInfo samplerInfo{.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter = VK_FILTER_LINEAR; //can be VK_FILTER_NEREAST (resutls in pixelatino)
    samplerInfo.minFilter = VK_FILTER_LINEAR; //can be VK_FILTER_NEREAST (resutls in pixelatino)
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    // what happens when we attemp to sample where there are not pixels
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; // can be repeat, clamp to eadge or clamp to border
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);
    samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;

    //color to be used when VK_SAMPLER_ADRESSS_MODE_BORDER is selected
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    // use uv coordinates in range (0,1) - true or (0,imageLimit) - false
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    // compare values against reference, this will be important while calculating PCF shadow
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    // mip mapping
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f; // can also be physicalDeviceProperties.limits.maxSamplerLodBias
    samplerInfo.minLod = 0.0;
    samplerInfo.maxLod = m_material->GetMaximalMipValue();

    if (vkCreateSampler(m_device, &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture sampler");
    }

    for (auto &material: m_material->GetTextures()) {
        material.second.sampler = m_textureSampler;
    }
}

void VulkanApp::CreateColorResources() {
    VkFormat colorFormat = m_swapChainImageFormat;

    ImageCreateInfo multisampleImageInfo{};
    multisampleImageInfo.physicalDevice = m_physicalDevice;
    multisampleImageInfo.logicalDevice = m_device;
    multisampleImageInfo.surface = m_sruface;
    multisampleImageInfo.format = colorFormat;
    multisampleImageInfo.width = m_swapChainExtent.width;
    multisampleImageInfo.height = m_swapChainExtent.height;
    multisampleImageInfo.mipLevels = 1;
    multisampleImageInfo.imageTiling = VK_IMAGE_TILING_OPTIMAL;
    multisampleImageInfo.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    multisampleImageInfo.memoryProperteis = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    multisampleImageInfo.sampleCount = m_msaaSamples;
    CreateImage(multisampleImageInfo, m_colorImage, m_colorImageMemory);

    m_colorImageView = GenerateImageView(m_device, m_colorImage, 1, colorFormat);
}

void VulkanApp::CreateCommandBuffers() {
    m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_comandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffer");
    }
}

void VulkanApp::CreateDepthResources() {
    VkFormat format = FindDepthFormat();

    ImageCreateInfo imageInfo{};
    imageInfo.physicalDevice = m_physicalDevice;
    imageInfo.logicalDevice = m_device;
    imageInfo.surface = m_sruface;
    imageInfo.format = format;
    imageInfo.size = (m_swapChainExtent.width * m_swapChainExtent.height);
    imageInfo.width = m_swapChainExtent.width;
    imageInfo.height = m_swapChainExtent.height;
    imageInfo.imageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.memoryProperteis = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    imageInfo.sampleCount = m_msaaSamples;

    CreateImage(imageInfo, m_depthImage, m_depthMemory);

    m_depthImageView = GenerateImageView(m_device, m_depthImage, 1, format, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void VulkanApp::CreateShaderStorageBuffer() {
    m_shaderStorageBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    m_shaderStorageBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);

    // Cretes random values between 0 and 1 with time as a seed
    std::default_random_engine rndEngine((unsigned) time(nullptr));
    std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

    std::vector<Particle> particles(PARTICLE_COUNT);
    for(auto& particle: particles) {
        float radius  = 0.25f*sqrt(rndDist(rndEngine));
        //scatter around entire circle
        float theta = rndDist(rndEngine)* 2 * 3.14159265358979323846;
        //from angle and radius to x and y
        float x = radius * cos(theta) * HEIGHT / WIDTH;
        float y = radius * sin (theta);

        particle.position = glm::vec2(x,y);
        particle.velocity = glm::normalize(glm::vec2(x,y))*0.00025f;
        particle.color = glm::vec4(rndDist(rndEngine),rndDist(rndEngine),rndDist(rndEngine),1.0f);
    }

    VkDeviceSize stagingBufferSize = PARTICLE_COUNT * sizeof(Particle);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    BufferCreateInfo bufferCreateInfo;
    bufferCreateInfo.physicalDevice = m_physicalDevice;
    bufferCreateInfo.logicalDevice = m_device;
    bufferCreateInfo.surface = m_sruface;
    bufferCreateInfo.size = stagingBufferSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    CreateBuffer(bufferCreateInfo, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, stagingBufferSize, 0, &data);
    memcpy(data, particles.data(),(size_t)stagingBufferSize);

    for(size_t i=0; i<MAX_FRAMES_IN_FLIGHT; i++) {
        //note the last bit flag, it is converting the buffer to be SSBO
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        bufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        CreateBuffer(bufferCreateInfo, m_shaderStorageBuffer[i], m_shaderStorageBufferMemory[i]);
        // copy from staging buffer to the acctual buffer on the GPU that acts like and SSBO
        CopyBuffer(m_device,m_transferQueue, m_transferCommandPool, stagingBuffer, m_shaderStorageBuffer[i], stagingBufferSize);
    }
}

void VulkanApp::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.3f, 0.3f, 0.3f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to start recording the command buffer");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapChainFrameBuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChainExtent;

    const float red = glm::abs(glm::sin(glfwGetTime()));

    VkClearValue clearValue = {{{0.3f, 0.3f, 0.3f, 1.0f}}};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;

    viewport.width = static_cast<float>(m_swapChainExtent.width);
    viewport.height = static_cast<float>(m_swapChainExtent.height);

    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissors{};
    scissors.offset = {0, 0};
    scissors.extent = m_swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissors);

    VkBuffer vertexBuffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
                            &m_descriptorSets[currentFrame], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer !");
    }
}

VkCommandBuffer VulkanApp::StartRecordingCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.commandPool = m_transferCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanApp::FlushCommandBuffer(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_transferQueue);
    vkFreeCommandBuffers(m_device, m_transferCommandPool, 1, &commandBuffer);
}

void VulkanApp::CreateSyncObjects() {
    m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS
        ) {
            throw std::runtime_error("Failed to create synchronization objects");
        }
    }
}

void VulkanApp::UpdateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.model = glm::translate(ubo.model, glm::vec3(0.0, -3.0f, 0.0f));
    ubo.model = glm::scale(ubo.model, glm::vec3(1.7f));
    ubo.projection = m_camera->getPojectionMatix();
    ubo.projection[1][1] *= -1;
    ubo.view = m_camera->getViewMatrix();
    ubo.camPos = m_camera->getPosition();
    ubo.normal = glm::transpose(glm::inverse(ubo.model));
    ubo.lightPos = m_lightPos;

    memcpy(m_uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

void VulkanApp::CleanupSwapChain() {
    for (auto frameBuffer: m_swapChainFrameBuffers) {
        vkDestroyFramebuffer(m_device, frameBuffer, nullptr);
    }

    for (auto imageView: m_swapChainImageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }


    vkDestroyImageView(m_device, m_depthImageView, nullptr);
    vkDestroyImage(m_device, m_depthImage, nullptr);
    vkFreeMemory(m_device, m_depthMemory, nullptr);

    vkDestroyImageView(m_device, m_colorImageView, nullptr);
    vkDestroyImage(m_device, m_colorImage, nullptr);
    vkFreeMemory(m_device, m_colorImageMemory, nullptr);


    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
}

void VulkanApp::RecreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    while (width == 0 || height == 0) {
        m_appNotifier.appStatus = IDLE;
        m_appNotifier.isDirty = true;
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_device);

    CleanupSwapChain();

    CreateSwapChain();
    CreateImageViews();
    CreateColorResources();
    CreateDepthResources();
    CreateFrameBuffers();
}

void VulkanApp::CreateLogicalDevice() {
    //finds queue family with graphics capabilities VK_QUEUE_GRAPHICS_BIT
    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice, m_sruface);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsAndComputeFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (auto queueFamily: uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtentions.size());
    createInfo.ppEnabledExtensionNames = deviceExtentions.data();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device !");
    }

    vkGetDeviceQueue(m_device, indices.graphicsAndComputeFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentationQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_transferQueue);
    vkGetDeviceQueue(m_device, indices.graphicsAndComputeFamily.value(), 0, &m_computeQueue);

    this->m_material = std::make_unique<Material>(m_device);
}

void VulkanApp::CreateSurface() {
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_sruface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
}

VkSampleCountFlagBits VulkanApp::GetMaxUsableSampleCount() {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

    VkSamplerCreateFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts
                                  &
                                  physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

std::vector<const char *> VulkanApp::GetRequiredExtentions() {
    uint32_t glfwExtentionsCount = 0;
    const char **glfwExtentions;
    glfwExtentions = glfwGetRequiredInstanceExtensions(&glfwExtentionsCount);

    std::vector<const char *> extensions(glfwExtentions, glfwExtentions + glfwExtentionsCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void VulkanApp::SetUpDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessanger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a debug messenger");
    } else
        std::cout << "Debug messenger created" << std::endl;
}

void VulkanApp::CleanUp() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(m_device, m_comandPool, nullptr);
    vkDestroyCommandPool(m_device, m_transferCommandPool, nullptr);

    CleanupSwapChain();

    vkDestroySampler(m_device, m_textureSampler, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
        vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);

        vkDestroyBuffer(m_device, m_deltaTimeUBOBuffer[i], nullptr);
        vkFreeMemory(m_device, m_deltaTimeUBOMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

    vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);

    vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
    vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
    m_material.reset();

    vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessanger, nullptr);
    }
    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_sruface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void VulkanApp::FrameBufferResizeCallback(GLFWwindow *window, int width, int height) {
    std::cout << "Resize x: " << width << "y: " << height << std::endl;
    auto app = reinterpret_cast<VulkanApp *>(glfwGetWindowUserPointer((window)));
    app->m_frameBufferResized = true;
    app->m_camera->processResize(width, height);
}

void VulkanApp::MousePositionCallback(GLFWwindow *window, double xpos, double ypos) {
    auto app = reinterpret_cast<VulkanApp *>(glfwGetWindowUserPointer((window)));
    auto pointerX = (float) xpos;
    auto pointerY = (float) ypos;
    if (app->m_isFirstMouse) {
        app->m_lastX = xpos;
        app->m_lastY = ypos;
        app->m_isFirstMouse = false;;
    }

    float xOffset = xpos - app->m_lastX;
    float yOffset = app->m_lastY - ypos; // Invert the sign here

    app->m_lastX = xpos;
    app->m_lastY = ypos;

    xOffset *= 0.01;
    yOffset *= 0.01;

    if (xOffset != 0.0 && app->m_isMousePressed) {
        app->m_camera->rotateAzimutn(xOffset);
    }

    if (yOffset != 0.0 && app->m_isMousePressed) {
        app->m_camera->rotatePolar(-yOffset);
    }
}

void VulkanApp::MouseClickCallback(GLFWwindow *window, int button, int action, int mods) {
    auto app = reinterpret_cast<VulkanApp *>(glfwGetWindowUserPointer((window)));
    GLFWcursor *hand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    GLFWcursor *cursor = glfwCreateStandardCursor(GLFW_CURSOR_NORMAL);
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            app->m_isMousePressed = true;
            glfwSetCursor(app->m_window, hand);
        } else if (action == GLFW_RELEASE) {
            app->m_isMousePressed = false;
            glfwSetCursor(app->m_window, cursor);
        }
    }
}

void VulkanApp::MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    auto app = reinterpret_cast<VulkanApp *>(glfwGetWindowUserPointer((window)));
    app->m_camera->zoom((float) yoffset);
}

void VulkanApp::ProcessKeyboardInput() {
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, true);


    const float lightSpeed = 0.8f; // adjust accordingly
    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
        m_lightPos.z += lightSpeed;
    if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
        m_lightPos.x -= lightSpeed;
    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
        m_lightPos.z -= lightSpeed;
    if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        m_lightPos.x += lightSpeed;
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        m_lightPos.y += lightSpeed;
    if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        m_lightPos.y -= lightSpeed;
}

void VulkanApp::GenerateGeometryVertices(GEOMETRY_TYPE geometryType) {
    m_geometryType = geometryType;
    switch (geometryType) {
        case CUBE: {
            vertices = VertexData::cubeVertices;
            indices = VertexData::cubeIndices;
            break;
        }
        case PLANE: {
            vertices = VertexData::planeVertices;
            indices = VertexData::planeIndices;
            break;
        }
        case SPHERE: {
            GenerateSphere(vertices, indices);
            break;
        }
        case MODEL: {
            LoadModel();
        }
    }
}


void VulkanApp::LoadModel() {
    //contains vertices, normals, uv all packed together
    tinyobj::attrib_t attrib;
    //contains all of the objects and their faces by keeping the index of the vertices loaded above
    std::vector<tinyobj::shape_t> shapes;
    //materials and textures per face
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVetices{};


    for (const auto &shape: shapes) {
        for (const auto &index: shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.uv = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2],
            };

            vertex.color = {1.0, 1.0, 1.0};

            //is the vertex unique ?
            if (uniqueVetices.count(vertex) == 0) {
                //add its index
                uniqueVetices[vertex] = static_cast<uint32_t>(vertices.size());
                //add vertex itself
                vertices.push_back(vertex);
            }
            //store only index
            indices.push_back(uniqueVetices[vertex]);
        }
    }
}


VkFormat VulkanApp::FindDepthFormat() {
    return FinsSupportedFormat(m_physicalDevice, m_device,
                               {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                               VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
