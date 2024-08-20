#version 460

layout (binding = 0) uniform UnifromBufferObject {
    vec3 camPos;
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normalMatix;
}ubo;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec3 color;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec3 cameraPosition;
layout (location = 3) out vec3 fragPos;

void main() {
    gl_Position = ubo.proj* ubo.view * ubo.model * vec4(inPosition,1.0);
    normal = vec3(ubo.normalMatix * vec4(inNormal, 1.0));
    color =  inColor;
    cameraPosition = ubo.camPos;
    fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
}