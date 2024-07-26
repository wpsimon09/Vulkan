//
// Created by wpsimon09 on 26/07/24.
//

#ifndef UTILS_HPP
#define UTILS_HPP
#include <vulkan/vulkan_core.h>

inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfor, const VkAllocationCallbacks* pAllocator,VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT" );
    if(func !=nullptr)
        return func(instance,pCreateInfor,pAllocator,pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

#endif //UTILS_HPP
