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

    uint32_t glfwExtentionCount;
    const char ** glfwExtentions;
    glfwExtentions = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);

    createInfo.enabledExtensionCount = glfwExtentionCount;
    createInfo.ppEnabledExtensionNames = glfwExtentions;

    //number of validation layers
    createInfo.enabledLayerCount = 0;

    if(vkCreateInstance(&createInfo, nullptr, &m_instance)!=VK_SUCCESS) {
        throw std::runtime_error("Failed to create intance ");
    }

    uint32_t extenstionsCount;
    //get cound ot extentions
    vkEnumerateInstanceExtensionProperties(nullptr, &extenstionsCount, nullptr);
    std::vector<VkExtensionProperties> extentions(extenstionsCount);
    //get the acctual extentions                    //layer   //num of extentios          //where to store them
    vkEnumerateInstanceExtensionProperties(nullptr, &extenstionsCount, extentions.data());

    std::cout << "available extensions:\n";

    for (const auto& extension : extentions) {
        std::cout << '\t' << extension.extensionName << '\t' << "V: " <<(float)extension.specVersion<< '\n';
    }
}

void HelloTriangle::MainLoop() {
    while(!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

void HelloTriangle::CleanUp() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}


