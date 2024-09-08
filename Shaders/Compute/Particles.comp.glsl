#version 460

layout(binding = 0) uniform ParameterUBOb{
    float deltaTime;
}ubo;

struct Particle{
    vec2 position;
    vec2 velocity;
    vec4 color;
};

layout(std140, binding = 1) readonly buffer ParticleSSBOIn{
    Particle particlesIn[];
};

layout(std140, binding = 2) readonly buffer ParticleSSBOOut{
    Particle particlesOut[];
};

void main() {

}