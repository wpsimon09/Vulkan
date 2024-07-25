//
// Created by wpsimon09 on 25/07/24.
//

#ifndef HELLOTRIANGLE_HPP
#define HELLOTRIANGLE_HPP

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <cstdlib>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class HelloTriangle {
public:
    void run();
private:
    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void CleanUp();

    GLFWwindow* m_window;
};

#endif //HELLOTRIANGLE_HPP
