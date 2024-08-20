#version 460

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 color;
layout (location = 1) in vec3 normal;

void main() {
    outColor = vec4(normal,1.0);
}