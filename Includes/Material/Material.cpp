//
// Created by wpsimon09 on 24/08/24.
//

#include "Material.hpp"

Material::Material(VkDevice &logicalDevic) {
    this->materials.insert(std::make_pair(TEXTURE_TYPE_ALBEDO, Texture{}));
    this->materials.insert(std::make_pair(TEXTURE_TYPE_ARM, Texture{}));
    this->materials.insert(std::make_pair(TEXTURE_TYPE_NORMAL, Texture{}));

    this->logicalDevice = logicalDevic;
}

Material::~Material() {
    for (auto texture: this->materials) {
        vkDestroyImageView(logicalDevice, texture.second.imageView, nullptr);
        vkDestroyImage(logicalDevice, texture.second.image, nullptr);
        vkFreeMemory(logicalDevice, texture.second.memory, nullptr);
    }
}
