#!/bin/bash

VULKAN_SDK_PATH="/home/wpsimon09/vulkan-sdk/1.3.290.0/x86_64/bin/glslc"
if [[ ! -x "$VULKAN_SDK_PATH" ]]; then
    echo "Error: Vulkan SDK compiler not found at $VULKAN_SDK_PATH"
    exit 1
fi

mkdir -p Compiled

for vert_shader in Vertex/*.vert; do
    if [[ -f "$vert_shader" ]]; then
        shader_name=$(basename "$vert_shader")
        echo "Compiling vertex shader: $shader_name"
        $VULKAN_SDK_PATH "$vert_shader" -o "Compiled/${shader_name%.vert}.spv"
        if [[ $? -eq 0 ]]; then
            echo "Compiled $vert_shader to Compiled/${shader_name%.vert}"
        else
            echo "Failed to compile $vert_shader"
        fi
    else
        echo "No vertex shaders found in Vertex directory"
    fi
done

for frag_shader in Fragment/*.frag; do
    if [[ -f "$frag_shader" ]]; then
        shader_name=$(basename "$frag_shader")
        echo "Compiling fragment shader: $shader_name"
        $VULKAN_SDK_PATH "$frag_shader" -o "Compiled/${shader_name%.frag}.spv"
        if [[ $? -eq 0 ]]; then
            echo "Compiled $frag_shader to Compiled/${shader_name%.frag}.spv"
        else
            echo "Failed to compile $frag_shader"
        fi
    else
        echo "No fragment shaders found in Fragment directory"
    fi
done