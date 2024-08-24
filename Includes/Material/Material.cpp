//
// Created by wpsimon09 on 24/08/24.
//

#include "Material.hpp"

Material::Material() {
    this->materials.insert(std::make_pair(TEXTURE_TYPE_ALBEDO, Texture{}));
    this->materials.insert(std::make_pair(TEXTURE_TYPE_ARM, Texture{}));
    this->materials.insert(std::make_pair(TEXTURE_TYPE_NORMAL, Texture{}));
}
