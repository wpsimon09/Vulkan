#version 436



layout(location = 0) in vec3 inParticlePosition;
layout(location = 1) in vec4 inParticleColour;

layout (binding = 0) uniform UnifromBufferObject {
    vec3 camPos;
    vec3 lightPosition;
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normalMatix;
}ubo;

layout(location = 0) out vec3 outFragColor;


void main() {
    gl_PointSize = 14.0;

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inParticlePosition,1.0);
    outFragColor = inParticleColour;
}