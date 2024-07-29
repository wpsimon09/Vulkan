//
// Created by wpsimon09 on 26/07/24.
//

#ifndef DEBUGINFOLOG_HPP
#define DEBUGINFOLOG_HPP
#include <iostream>
#include <vulkan/vulkan_core.h>

inline static void GetSeverity(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
    switch (severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            std::cerr<<"Vulkan::Error::Severity - \n";
            std::cout<<"Diganostic message"<<std::endl;
            break;;
        }case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            std::cerr<<"Vulkan::Error::Severity - \n";
            std::cout<<"Information message"<<std::endl;
            break;;
        }case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            std::cerr<<"Vulkan::Error::Severity - \n";
            std::cout<<"Behavioural message"<<std::endl;
            break;;
        }case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            std::cerr<<"Vulkan::Error::Severity - \n";
            std::cout<<"Crash message"<<std::endl;
            break;;
        }default: {
            std::cout<<"Message has no severity"<<std::endl;
            break;
        }
    }
}

inline static void GetMessageType(VkDebugUtilsMessageTypeFlagsEXT messageType) {

    switch (messageType) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: {
            std::cerr<<"Vulkan::Error::MessageType - \n";
            std::cout<<"General message"<<std::endl;
            break;;
        }case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            std::cerr<<"Vulkan::Error::MessageType - \n";
            std::cout<<"Specification víolation or possible mistake"<<std::endl;
            break;;
        }case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            std::cerr<<"Vulkan::Error::MessageType - \n";
            std::cout<<"Non optimal use of vulkan"<<std::endl;
            break;
        }default: {
            std::cout<<"No message type"<<std::endl;
            break;
        }
    }
}

#endif //DEBUGINFOLOG_HPP
