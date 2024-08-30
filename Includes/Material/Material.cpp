//
// Created by wpsimon09 on 24/08/24.
//

#include "Material.hpp"

Material::Material(VkDevice &logicalDevic) {
    this->m_materials.insert(std::make_pair(TEXTURE_TYPE_ALBEDO, Texture{}));
    this->m_materials.insert(std::make_pair(TEXTURE_TYPE_ARM, Texture{}));
    this->m_materials.insert(std::make_pair(TEXTURE_TYPE_NORMAL, Texture{}));

    this->m_logicalDevice = logicalDevic;
}

std::vector<VkDescriptorSetLayoutBinding> Material::GetLayoutBindings(int startsFrom) {
    int i = startsFrom;
    std::vector<VkDescriptorSetLayoutBinding> layoutBinidngs;
    for (auto &texture: this->m_materials) {
        VkDescriptorSetLayoutBinding imageSamplerLayoutBinding{};
        imageSamplerLayoutBinding.binding = i;
        imageSamplerLayoutBinding.descriptorCount = 1;
        imageSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageSamplerLayoutBinding.pImmutableSamplers = nullptr;
        imageSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        texture.second.binding = i;

        layoutBinidngs.emplace_back(imageSamplerLayoutBinding);
        i++;
    }

    return layoutBinidngs;
}

VkDescriptorPoolSize Material::GetDescriptorPoolSize(uint32_t MAX_FRAMES_IN_FLIGHT) {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = m_materials.size() * MAX_FRAMES_IN_FLIGHT;
    return poolSize;
}

std::vector<VkWriteDescriptorSet> Material::GetDescriptorWrites(VkDescriptorSet descriptorSet) {
    std::vector<VkWriteDescriptorSet> descritorWrites;
    descritorWrites.resize(static_cast<uint32_t>(m_materials.size()));
    m_descriptorImageInfos.resize(static_cast<uint32_t>(m_materials.size()));
    int i = 0;
    for (auto &texture: m_materials) {

        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = texture.second.sampler;
        imageInfo.imageView = texture.second.imageView;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        m_descriptorImageInfos[i] = imageInfo;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = texture.second.binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.pImageInfo = &m_descriptorImageInfos[i];
        descriptorWrite.pBufferInfo = nullptr;
        descriptorWrite.pTexelBufferView = nullptr;
        descriptorWrite.pNext = nullptr;

        descritorWrites[i] = descriptorWrite;

        i++;
    }

    return descritorWrites;
}

uint32_t Material::GetMaximalMipValue() {
    int smallestMipCount = m_materials[TEXTURE_TYPE_ALBEDO].maxMipLevels;
    for (auto &texture: m_materials) {
        if(texture.second.maxMipLevels < smallestMipCount && texture.second.maxMipLevels > 0) {
            smallestMipCount = texture.second.maxMipLevels;
        }
    }
}



Material::~Material() {
    for (auto texture: this->m_materials) {
        vkDestroyImageView(m_logicalDevice, texture.second.imageView, nullptr);
        vkDestroyImage(m_logicalDevice, texture.second.image, nullptr);
        vkFreeMemory(m_logicalDevice, texture.second.memory, nullptr);
    }
}
