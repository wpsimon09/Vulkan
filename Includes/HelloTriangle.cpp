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


