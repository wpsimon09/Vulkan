#version 460


layout(location = 0) in vec3 inParticlePosition;
layout(location = 2) in vec4 inParticleColour;

layout (binding = 0) uniform UnifromBufferObject {
    vec3 camPos;
    vec3 lightPosition;
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 normalMatix;
}ubo;

layout(location = 0) out vec4 outFragColor;


void main() {
    gl_PointSize = 14.0;

    //ubo.proj * ubo.view * ubo.model *
    gl_Position =  vec4(inParticlePosition.xy,1.0,1.0);
    outFragColor = inParticleColour;
}