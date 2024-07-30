#version 460
#include ""

layout (location = 0) out vec4 outColor;
layout (location = 0) in vec3 color;

void main() {
    outColor = vec4(color,1.0);
}