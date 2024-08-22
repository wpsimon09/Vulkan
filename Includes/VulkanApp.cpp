//
// Created by wpsimon09 on 25/07/24.
//

#include "VulkanApp.hpp"

#include <emmintrin.h>
#include <unistd.h>


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
                "\n File:\t" <<__FILE__<<
                "\n Message:\t"     <<pCallbackData ->pMessage <<
                "\n Message ID:\t"  <<pCallbackData->messageIdNumber<<
                "\n Object name:\t" <<pCallbackData->pObjects->pObjectName<<
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
    glfwSetFramebufferSizeCallback(m_window,FrameBufferResizeCallback);
    glfwSetCursorPosCallback(m_window,MousePositionCallback);
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
    CreateRenderPass();
    GenerateGeometryVertices(PLANE);
    CreateUniformBuffers();
    CreateDescriptorSetLayout();
    CreateDescriptorPool();
    CreateDescriptorSet();
    CreateGraphicsPipeline();
    CreateFrameBuffers();
    CreateTextureImage();
    CreateCommandPool();
    CreateVertexBuffers();
    CreateIndexBuffers();
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
    appInfo.pApplicationName = "Hello triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    appInfo.pNext = nullptr;

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
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR ) {
        RecreateSwapChain();
        return;
    }else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
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

    VkSemaphore signalSemaphores [] = {m_renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[currentFrame]) != VK_SUCCESS) {
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

    result = vkQueuePresentKHR(m_presentationQueue,&presentInfo);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_frameBufferResized) {
        RecreateSwapChain();
        m_frameBufferResized = false;
    }else if(result != VK_SUCCESS) {
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
    uint32_t queueFamilyIndecies[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    //if presentation and graphics queue family are the same use the exlusive mode
    //otherwise use concurent mode
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndecies;
    }
    else {
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

    if(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain");
    }
    std::cout<<"Swapchain created !\n";

    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

    if(m_swapChainImages.empty())
        throw std::runtime_error("No swap chain images retrieved");
    std::cout<<"Retrieved "<<m_swapChainImages.size()<<" swap chain images\n";
}

void VulkanApp::CreateImageViews() {
    m_swapChainImageViews.resize(m_swapChainImages.size());
    for(size_t i = 0; i<m_swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(m_device,&createInfo,nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image views ");
        }
    }
}

void VulkanApp::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    //before render call colou and depth
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //after render call colour and depth
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //before render call
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //after render call
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    //---------
    // SUB PASS
    //---------
    VkAttachmentReference colorAttachmentRef{};
    //reference to the imaginary array of VkAttachmentDescription (we only have one)
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subPass{};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //reference to the colour attachemnt
    subPass.colorAttachmentCount = 1;
    subPass.pColorAttachments = &colorAttachmentRef;

    //--------------------
    // SUB PASS DEPENDENCY
    //--------------------
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    //subpass we are going to use since we only have one we will pass 0
    //dst subpass must be heigher than srcSubpass, only exception is if src is VK_SUBPASS_EXTERNAL
    dependency.dstSubpass = 0;
    //dependecy start
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //we are not targeting any memmory so 0
    dependency.srcAccessMask = 0;

    //dependency end
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //we want to access colour attachemnt so that we can write to it
    dependency.dstAccessMask= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subPass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;


    if(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass");
    }

}

void VulkanApp::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;

    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;
    layoutInfo.pNext = nullptr;

    if(vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    };
}

void VulkanApp::CreateDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.poolSizeCount =1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool");
    }

}

void VulkanApp::CreateDescriptorSet() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();
    m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

    if(vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptors sets");
    }

    for(size_t i = 0; i<MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet descriptorWrite{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptorWrite.dstSet = m_descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;

        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;

        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr;
        descriptorWrite.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);
    }
}

void VulkanApp::CreateGraphicsPipeline() {

    //----------------
    // SHADER CREATION
    //----------------
    auto vertShaderCode = readFile("Shaders/Compiled/TriangleVertex.spv");
    auto fragmentShaderCode = readFile("Shaders/Compiled/TriangleFragment.spv");
    std::cout<<"Shader read sucessfuly\n";

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
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

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
    inputAssemblyCreateInfo.topology = m_geometryType == SPHERE ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    //----------
    // VIEW PORT
    //----------
    VkViewport viewPort{};
    viewPort.x = 0.0f;
    viewPort.y = 0.0f;
    viewPort.width = (float)m_swapChainExtent.width;
    viewPort.height = (float)m_swapChainExtent.height;
    viewPort.minDepth = 0.0f;
    viewPort.maxDepth = 1.0f;

    //---------
    // SCISSORS
    //---------
    VkRect2D scissor{};
    scissor.offset = {0,0};
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
    dynmicStateVPandScissors.dynamicStateCount = static_cast <uint32_t>(dynamicStates2.size());
    dynmicStateVPandScissors.pDynamicStates= dynamicStates2.data();

    VkPipelineViewportStateCreateInfo viewPortState{};
    viewPortState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewPortState.viewportCount = 1;
    viewPortState.scissorCount=1;

    //-----------
    // RASTERIZER
    //-----------
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode =  m_geometryType == PLANE ? VK_CULL_MODE_NONE :VK_CULL_MODE_BACK_BIT;
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
    multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleCreateInfo.minSampleShading = 1.0f;
    multisampleCreateInfo.pSampleMask = nullptr;
    multisampleCreateInfo.alphaToCoverageEnable= VK_FALSE;
    multisampleCreateInfo.alphaToOneEnable =VK_FALSE;

    //-----------------------
    // DEPTH AND STENCIL TEST
    //-----------------------
    //later|

    //----------------
    // COLOUR BLENDING
    //----------------
    VkPipelineColorBlendAttachmentState colourBlendAttachmentCreateInfo{};
    colourBlendAttachmentCreateInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
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

    if(vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
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
    pipelineInfo.pRasterizationState= &rasterizerCreateInfo;
    pipelineInfo.pMultisampleState = &multisampleCreateInfo;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colourBlendCreateInfo;
    pipelineInfo.pDynamicState = &dynamicStateCreateInfo;
    //pipeline layout for uniforms
    pipelineInfo.layout = m_pipelineLayout;
    //render pass
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    //pipeline handle
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex  = -1;

    if(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE,1,&pipelineInfo, nullptr, &m_graphicsPipeline)) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }else {
        std::cout<<"Graphics pipeline created sucessfully !\n";
    }


    vkDestroyShaderModule(m_device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(m_device, fragmentShaderModule, nullptr);

}

void VulkanApp::CreateFrameBuffers() {
    m_swapChainFrameBuffers.resize(m_swapChainImageViews.size());

    for(size_t i=0; i<m_swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            m_swapChainImageViews[i]
        };
        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = m_renderPass;
        frameBufferInfo.attachmentCount = 1;
        frameBufferInfo.pAttachments = attachments;
        frameBufferInfo.width = m_swapChainExtent.width;
        frameBufferInfo.height = m_swapChainExtent.height;
        frameBufferInfo.layers = 1;

        if(vkCreateFramebuffer(m_device, &frameBufferInfo, nullptr, &m_swapChainFrameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create frame buffers from swap chain images");
        }
    }
}

void VulkanApp::CreateTextureImage() {
    int texWidth, texHeight, texChanels;
    stbi_uc* pixels = stbi_load("Textures/wood.png", &texWidth, &texHeight, &texChanels, STBI_rgb_alpha);

    if(!pixels) {
        throw std::runtime_error("Failed to load textue");
    }

    // times 4 becaus   e we have RGBA
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    VkBuffer stagingImageBuffer;
    VkDeviceMemory stagingImageMemory;

    BufferCreateInfo bufferInfo;
    bufferInfo.physicalDevice = m_physicalDevice;
    bufferInfo.logicalDevice = m_device;
    bufferInfo.surface = m_sruface;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    bufferInfo.size = imageSize;

    CreateBuffer(bufferInfo, stagingImageBuffer, stagingImageMemory);

    void *data;
    vkMapMemory(m_device, stagingImageMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device, stagingImageMemory);

    stbi_image_free(pixels);

    ImageCreateInfo imageCreateInfo {};
    imageCreateInfo.physicalDevice = m_physicalDevice;
    imageCreateInfo.logicalDevice = m_device;
    imageCreateInfo.surface = m_sruface;
    imageCreateInfo.width = texWidth;
    imageCreateInfo.height = texHeight;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageCreateInfo.imageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.size = imageSize;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.memoryProperteis = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    CreateImage(imageCreateInfo, m_textureImage, m_textureImageMemory);

    ImageLayoutDependencyInfo dependencyInfo{};
    dependencyInfo.commandPool = m_transferCommandPool;
    dependencyInfo.logicalDevice = m_device;
    dependencyInfo.transformQueue = m_transferQueue;
    TransferImageLayout(dependencyInfo, m_textureImage, imageCreateInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    CopyBufferToImage(dependencyInfo,stagingImageBuffer, m_textureImage, static_cast<uint32_t>(texWidth),static_cast<uint32_t>(texHeight));

    TransferImageLayout(dependencyInfo, m_textureImage, imageCreateInfo.format,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void VulkanApp::CreateCommandPool() {
    //retrieve all queue families from the GPU
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice, m_sruface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //use the graphics family for the drawing
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_comandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool !");
    }

    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();
    if(vkCreateCommandPool(m_device,&poolInfo, nullptr, &m_transferCommandPool) != VK_SUCCESS) {
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

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferInfo.size);
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
    CopyBuffer(m_device, m_transferQueue, m_transferCommandPool,stagingBuffer, m_vertexBuffer,bufferInfo.size);

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
    void* data;
    vkMapMemory(m_device, stagingMemory, 0, bufferCreateInfo.size,0, &data);
    memcpy(data, indices.data(), (size_t)bufferCreateInfo.size);
    vkUnmapMemory(m_device, stagingMemory);

    //create index buffer
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    CreateBuffer(bufferCreateInfo, m_indexBuffer, m_indexBufferMemory);

    //copy staging buffer to the index buffer
    CopyBuffer(m_device ,m_transferQueue, m_transferCommandPool, stagingBuffer, m_indexBuffer, bufferCreateInfo.size);

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

    for(size_t i=0; i< MAX_FRAMES_IN_FLIGHT; i++) {
        CreateBuffer(bufferInfo, m_uniformBuffers[i], m_uniformBuffersMemory[i]);

        vkMapMemory(m_device, m_uniformBuffersMemory[i], 0, bufferInfo.size, 0, &m_uniformBuffersMapped[i]);
    }
}

void VulkanApp::CreateCommandBuffers() {
    m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_comandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if(vkAllocateCommandBuffers(m_device,&allocInfo,m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffer");
    }
}

void VulkanApp:: RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if(vkBeginCommandBuffer(commandBuffer, &beginInfo)!=VK_SUCCESS) {
        throw std::runtime_error("Failed to start recording the command buffer");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapChainFrameBuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0,0};
    renderPassInfo.renderArea.extent = m_swapChainExtent;

    const float red = glm::abs(glm::sin(glfwGetTime()));

    VkClearValue clearValue = {{{0.3f, 0.3f, 0.3f,1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

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
    scissors.offset = {0,0};
    scissors.extent = m_swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissors);

    VkBuffer vertexBuffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[currentFrame], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer,static_cast<uint32_t>(indices.size()),1,0,0,0);

    vkCmdEndRenderPass(commandBuffer);

    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer !");
    }
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

    for(size_t i = 0; i< MAX_FRAMES_IN_FLIGHT;i++) {
        if(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
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
    ubo.model = glm::scale(ubo.model, glm::vec3(100.7f));
    ubo.projection = m_camera->getPojectionMatix();
    ubo.projection[1][1] *= -1;
    ubo.view = m_camera->getViewMatrix();
    ubo.camPos = m_camera->getPosition();
    ubo.normal = glm::transpose(glm::inverse(ubo.model));

    memcpy(m_uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

}

void VulkanApp::CleanupSwapChain() {
    for (auto frameBuffer: m_swapChainFrameBuffers) {
        vkDestroyFramebuffer(m_device, frameBuffer, nullptr);
    }

    for (auto imageView: m_swapChainImageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
}

void VulkanApp::RecreateSwapChain() {

    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    while(width == 0 || height == 0) {
        m_appNotifier.appStatus = IDLE;
        m_appNotifier.isDirty = true;
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_device);

    CleanupSwapChain();

    CreateSwapChain();
    CreateImageViews();
    CreateFrameBuffers();
}

void VulkanApp::CreateLogicalDevice() {
    //finds queue family with graphics capabilities VK_QUEUE_GRAPHICS_BIT
    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice, m_sruface);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

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

    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentationQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_transferQueue);
}

void VulkanApp::CreateSurface() {
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_sruface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
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
    for(size_t i  =0; i< MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(m_device, m_comandPool, nullptr);
    vkDestroyCommandPool(m_device, m_transferCommandPool, nullptr);
    CleanupSwapChain();

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
        vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

    vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
    vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);

    vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
    vkFreeMemory(m_device, m_indexBufferMemory, nullptr);


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
    std::cout<<"Resize x: "<<width<<"y: "<<height<<std::endl;
    auto app = reinterpret_cast<VulkanApp*>(glfwGetWindowUserPointer((window)));
    app->m_frameBufferResized = true;
    app->m_camera->processResize(width, height);
}

void VulkanApp::MousePositionCallback(GLFWwindow *window, double xpos, double ypos) {
    auto app = reinterpret_cast<VulkanApp*>(glfwGetWindowUserPointer((window)));
    auto pointerX = (float)xpos;
    auto pointerY = (float)ypos;
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

        if (yOffset != 0.0 && app->m_isMousePressed ) {
            app->m_camera->rotatePolar(-yOffset);
        }

}

void VulkanApp::MouseClickCallback(GLFWwindow *window, int button, int action, int mods) {
    auto app = reinterpret_cast<VulkanApp*>(glfwGetWindowUserPointer((window)));
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
    auto app = reinterpret_cast<VulkanApp*>(glfwGetWindowUserPointer((window)));
    app->m_camera->zoom((float) yoffset);
}

void VulkanApp::GenerateGeometryVertices(GEOMETRY_TYPE geometryType) {
    m_geometryType = geometryType;
    switch(geometryType) {
        case CUBE: {
            vertices = VertexData::cubeVertices;
            indices = VertexData::cubeIndices;
            break;
        }
        case PLANE: {
            vertices = VertexData::planeVertices;
            indices = VertexData::planeIndices;
            break;
        }case SPHERE: {
            GenerateSphere(vertices, indices);
            break;
        }
    }
}
