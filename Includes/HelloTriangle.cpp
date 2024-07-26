//
// Created by wpsimon09 on 25/07/24.
//

#include "HelloTriangle.hpp"


void HelloTriangle::run() {
    InitWindow();

    InitVulkan();
    MainLoop();

    CleanUp();

}

void HelloTriangle::InitWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangle::InitVulkan() {
    CreateInstance();
}

void HelloTriangle::CreateInstance() {
    if(enableValidationLayers && !this->CheckValidationLayerSupport()) {
        throw std::runtime_error("Requested validation layers were not found");
    }else {
        std::cout<<"Valiation layers found\n";
    }

    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_3;
    //extention callback
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    //reference to application info
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    uint32_t glfwExtentionCount;
    const char ** glfwExtentions;
    glfwExtentions = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);

    createInfo.enabledExtensionCount = glfwExtentionCount;
    createInfo.ppEnabledExtensionNames = glfwExtentions;

    //number of validation layers
    createInfo.enabledLayerCount = 0;

    if(vkCreateInstance(&createInfo, nullptr, &m_instance)!=VK_SUCCESS) {
        throw std::runtime_error("Failed to create intance \n");
    }else {
        std::cout<<"Vulkan instance created successfuly \n";
    }

    uint32_t extenstionsCount;
    //get count of extentions
    vkEnumerateInstanceExtensionProperties(nullptr, &extenstionsCount, nullptr);
    std::vector<VkExtensionProperties> extentions(extenstionsCount);

    //get the acctual extentions                    //layer   //num of extentios          //where to store them
    vkEnumerateInstanceExtensionProperties(nullptr, &extenstionsCount, extentions.data());

    std::cout << "available extensions:\n";

    int i = 0;

    for (const auto& extension : extentions) {
        std::cout << '\t' << extension.extensionName << '\t' << "V: " <<extension.specVersion<< '\n';
    }

}

bool HelloTriangle::CheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName: validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties: availableLayers) {
            std::cout<<'\t'<<layerProperties.layerName<< '\n';
            if(strcmp(layerName,layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if(!layerFound)
            return false;
    }
    return true;
}

void HelloTriangle::MainLoop() {
    while(!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

void HelloTriangle::CleanUp() {
    vkDestroyInstance(m_instance, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
}


