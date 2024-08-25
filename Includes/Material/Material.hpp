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
};

enum TEXTURE_TYPE {
    TEXTURE_TYPE_ALBEDO = 0,
    TEXTURE_TYPE_ARM = 1,
    TEXTURE_TYPE_NORMAL = 2,
};

class Material {
public:
    Material(VkDevice &logicalDevic);

    std::map<TEXTURE_TYPE, Texture> &GetTextures(){return this->materials;};

    Texture getAlbedo() {return materials[TEXTURE_TYPE_ALBEDO];};
    Texture getArm() {return materials[TEXTURE_TYPE_ARM];};
    Texture getNormal() {return materials[TEXTURE_TYPE_NORMAL];};

    ~Material();
private:
    VkDevice logicalDevice;
    std::map<TEXTURE_TYPE,Texture> materials;
};



#endif //MATERIAL_HPP
