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


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

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
private:
    void InitWindow();

    void InitVulkan();
    void CreateInstance();
    bool CheckValidationLayerSupport();
    void MainLoop();

    void CleanUp();

    GLFWwindow* m_window;
    VkInstance m_instance;
};

#endif //HELLOTRIANGLE_HPP
