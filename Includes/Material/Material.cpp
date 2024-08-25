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

std::vector<VkDescriptorSetLayoutBinding> Material::GetLayoutBindings(int startsFrom) {
    int i = startsFrom;
    std::vector<VkDescriptorSetLayoutBinding> layoutBinidngs;
    for (auto texture: this->materials) {
        VkDescriptorSetLayoutBinding imageSamplerLayoutBinding{};
        imageSamplerLayoutBinding.binding = i;
        imageSamplerLayoutBinding.descriptorCount = 1;
        imageSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageSamplerLayoutBinding.pImmutableSamplers = nullptr;
        imageSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        layoutBinidngs.emplace_back(imageSamplerLayoutBinding);
        i++;
    }

    return layoutBinidngs;
}

VkDescriptorPoolSize Material::GetDescriptorPoolSize(uint32_t MAX_FRAMES_IN_FLIGHT) {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = materials.size() * MAX_FRAMES_IN_FLIGHT;
    return poolSize;
}




Material::~Material() {
    for (auto texture: this->materials) {
        vkDestroyImageView(logicalDevice, texture.second.imageView, nullptr);
        vkDestroyImage(logicalDevice, texture.second.image, nullptr);
        vkFreeMemory(logicalDevice, texture.second.memory, nullptr);
    }
}
