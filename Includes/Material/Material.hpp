//
// Created by wpsimon09 on 24/08/24.
//

#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include <map>
#include <vector>
#include <vulkan/vulkan_core.h>

struct Texture {
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;
    VkSampler sampler;
    uint32_t binding;
    uint32_t maxMipLevels;
};

enum TEXTURE_TYPE {
    TEXTURE_TYPE_ALBEDO = 0,
    TEXTURE_TYPE_ARM = 1,
    TEXTURE_TYPE_NORMAL = 2,
};

class Material {
public:
    Material(VkDevice &logicalDevic);

    std::map<TEXTURE_TYPE, Texture> &GetTextures(){return this->m_materials;};

    std::vector<VkDescriptorSetLayoutBinding> GetLayoutBindings(int startsFrom = 0);

    VkDescriptorPoolSize GetDescriptorPoolSize(uint32_t MAX_FRAMES_IN_FLIGHT);

    std::vector<VkWriteDescriptorSet> GetDescriptorWrites(VkDescriptorSet descriptorSet);

    uint32_t GetMaximalMipValue();

    Texture getAlbedo() {return m_materials[TEXTURE_TYPE_ALBEDO];};
    Texture getArm() {return m_materials[TEXTURE_TYPE_ARM];};
    Texture getNormal() {return m_materials[TEXTURE_TYPE_NORMAL];};

    ~Material();
private:
    std::vector<VkDescriptorImageInfo> m_descriptorImageInfos;
    VkDevice m_logicalDevice;
    std::map<TEXTURE_TYPE,Texture> m_materials;
};



#endif //MATERIAL_HPP
