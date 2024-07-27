//
// Created by wpsimon09 on 26/07/24.
//

#ifndef DEBUGINFOLOG_HPP
#define DEBUGINFOLOG_HPP
#include <iostream>
#include <vulkan/vulkan_core.h>

inline static void GetSeverity(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
    std::cerr<<"Vulkan::Error::Severity - \n";
    switch (severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            std::cout<<"Diganostic message"<<std::endl;
            break;;
        }case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            std::cout<<"Information message"<<std::endl;
            break;;
        }case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            std::cout<<"Behavioural message"<<std::endl;
            break;;
        }case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            std::cout<<"Crash message"<<std::endl;
            break;;
        }default: {
            std::cout<<"Message has no severity"<<std::endl;
            break;
        }
    }
}

inline static void GetMessageType(VkDebugUtilsMessageTypeFlagsEXT messageType) {
    std::cerr<<"Vulkan::Error::MessageType - \n";

    switch (messageType) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: {
            std::cout<<"General message"<<std::endl;
            break;;
        }case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            std::cout<<"Specification víolation or possible mistake"<<std::endl;
            break;;
        }case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            std::cout<<"Non optimal use of vulkan"<<std::endl;
            break;
        }default: {
            std::cout<<"No message type"<<std::endl;
            break;
        }
    }
}

#endif //DEBUGINFOLOG_HPP
