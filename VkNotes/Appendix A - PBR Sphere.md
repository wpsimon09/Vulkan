
To create the sphere using PBR i first moved `vertices` and `indecies` to be members of the `HelloTriangle` class.

Than I extended the uniform buffer struct to contain `NormalMatrix` and `CameraPosition`

```c++
struct UniformBufferObject {  
    alignas(16)glm::vec3 camPos;  
    alignas(16)glm::mat4 model;  
    alignas(16)glm::mat4 view;  
    alignas(16)glm::mat4 projection;  
    alignas(16)glm::mat4 normal;  
};
```

Than I have extended the `Vertex` struct to contain normal vector as `Vertex` attribute

```c++
struct Vertex {  
    glm::vec3 pos;  
    glm::vec3 color;  
    glm::vec3 normal;

	static std::array<VkVertexInputAttributeDescription,3> getAttributeDescriptions() {  
    std::array<VkVertexInputAttributeDescription,3> attributeDescriptions;  
    //position
	----
	//color
	----

    //which vertex array binding to use  
    attributeDescriptions[2].binding = 0;  
    //location in shader  
    attributeDescriptions[2].location = 2;  
    //vec2 has 2 32-bit float components  
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;  
    //offset to the position  
    attributeDescriptions[2].offset = offsetof(Vertex, normal);  
  
    return attributeDescriptions;  
}

```

To generate everything i have created the following function 

```c++
static inline void GenerateSphere(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices) {  
    const unsigned int X_SEGMENTS = 64;  
    const unsigned int Y_SEGMENTS = 64;  
    const float PI = 3.14159265359f;  
    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)  
    {  
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)  
        {  
            Vertex tempVertex;  
            float xSegment = (float)x / (float)X_SEGMENTS;  
            float ySegment = (float)y / (float)Y_SEGMENTS;  
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);  
            float yPos = std::cos(ySegment * PI);  
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);  
  
            tempVertex.pos = glm::vec3(xPos, yPos, zPos);  
            tempVertex.normal = glm::vec3(xPos, yPos, zPos);  
  
            vertices.push_back(tempVertex);  
        }  
    }  
  
    bool oddRow = false;  
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)  
    {  
        if (!oddRow)  
        {  
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)  
            {  
                indices.push_back(y * (X_SEGMENTS + 1) + x);  
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);  
            }  
        }  
        else  
        {  
            for (int x = X_SEGMENTS; x >= 0; --x)  
            {  
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);  
                indices.push_back(y * (X_SEGMENTS + 1) + x);  
            }  
        }  
        oddRow = !oddRow;  
    }  
  
}
```

Then I have adjusted indices to be `uint32_f` as this sphere has a lot of unique vertices. This needed to be adjusted in multiple places

1. During index buffer creation 
```c++
BufferCreateInfo bufferCreateInfo{};  
bufferCreateInfo.physicalDevice = m_physicalDevice;  
bufferCreateInfo.logicalDevice = m_device;  
bufferCreateInfo.surface = m_sruface;  
  
bufferCreateInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;  
bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;  

//here
bufferCreateInfo.size = sizeof(uint32_t) * indices.size();
```

2. During recording of command buffer
```c++
vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
```

Since sphere generation is assuming that we are drawing triangle strips we have to tell it to the graphics pipeline. More specifically in the `topology` field of `VkPipelineInputAssemblyStateCreateInfo` struct

```c++
//-------------------  
// PRIMITIVE TOPOLOGY  
//-------------------  
VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};  
inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;  
inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;  
inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
```

Now i have altered shaders to receive correct information to calculate the lighting. That is `cameraPosition` from the `UnifromBuffer` and additional vertex attribute for the normals 

**Vertex shader**
```c++
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
```

And to calculate PBR I have used GGX BRDF model.
## Fragment shader

```c++
#version 460  
  
layout (location = 0) out vec4 outColor;  
  
layout (location = 0) in vec3 color;  
layout (location = 1) in vec3 normal;  
layout (location = 2) in vec3 cameraPosition;  
layout (location = 3) in vec3 fragPos;  
  
  
float roughness = 0.4;  
float metlaness = 0.7;  
float ao = 0.2;  
vec3 albedo = vec3(0.7, 0.7, 0.7);  
  
vec3 lightPos = vec3(0.0f, 4.0f, -4.0f);  
vec3 lightColor = vec3(4.0);  
  
const float PI = 3.14159265359;  
  
vec3 FresnelShlick(float cosTheta, vec3 F0)  
{  
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);  
}  
  
float DistributionFunctionGGX(vec3 N, vec3 H, float roughtness)  
{  
    float a = roughness * roughtness;  
    float a2 = a*a;  
    float NdotH = max(dot(N,H),0.0);  
    float NdotH2 = NdotH * NdotH;  
  
    //top part of fraction  
    float num = a2;  
  
    //bottom part of the fraction  
    float denom = (NdotH2 * (a2 -1.0) + 1.0);  
    denom = PI * denom * denom;  
  
    return num/denom;  
}  
  
  
float GeometryShclickGGX(float NdotVorL, float roughness)  
{  
    float r = (roughness + 1.0);  
    //roughtness parameter based on type of light  
    float k = (r * r)/8;  
  
    //top part of the fraction  
    float num = NdotVorL;  
  
    //bottom part of the fraction  
    float denom = NdotVorL * (1.0 - k) + k;  
  
    return num/denom;  
}  
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)  
{  
    //for view direction  
    float NdotV = max(dot(N,V),0.0);  
    float ggx2 = GeometryShclickGGX(NdotV, roughness);  
  
    //for light direction  
    float NdotL = max(dot(N,L), 0.0);  
    float ggx1 = GeometryShclickGGX(NdotL, roughness);  
  
    return ggx1 * ggx2;  
}  
  
void main() {  
    vec3 N = normalize(normal);  
    vec3 V = normalize(cameraPosition - fragPos);  
  
    vec3 F0 = vec3(0.04);  
    F0 = mix(F0, albedo, metlaness);  
  
    vec3 Lo = vec3(0.0);  
  
    vec3 L = normalize(lightPos - fragPos);  
  
    vec3 H = normalize(V + L);  
  
    float NDF = DistributionFunctionGGX(N,H, roughness);  
    float G = GeometrySmith(N,V,L, roughness);  
    vec3 F = FresnelShlick(max(dot(H, V), 0.0), F0);  
  
    vec3 numerator = NDF * G * F;  
    float denominator = 4.0 * max(dot(N,V), 0.0) * max(dot(N,L), 0.0);  
  
    vec3 specular = numerator / max(denominator, 0.001);  
  
    vec3 kS = F;  
    vec3 kD = vec3(1.0) - kS;  
  
    kD *= 1.0 - metlaness;  
  
    float NdotL = max(dot(N,L), 0.0);  
    Lo += (kD * albedo /PI + specular) * lightColor * NdotL;  
  
    vec3 ambient = vec3(0.3) * albedo * ao;  
    vec3 color = ambient + Lo;  
  
    outColor = vec4(color,1.0);  
}
```

Lastly during sending uniforms to the shader I have calculated the Normal matrix and passed it to the Uniform buffer together with the current camera position.

```c++
void HelloTriangle::UpdateUniformBuffer(uint32_t currentImage) {  
    UniformBufferObject ubo{};  
    ubo.model = glm::mat4(1.0f);  
    ubo.model = glm::scale(ubo.model, glm::vec3(2.7f));  
    ubo.projection = m_camera->getPojectionMatix();  
    ubo.projection[1][1] *= -1;  
    ubo.view = m_camera->getViewMatrix();  
    ubo.camPos = m_camera->getPosition();  
    ubo.normal = glm::transpose(glm::inverse(ubo.model));  
  
    memcpy(m_uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));  
  
}
```