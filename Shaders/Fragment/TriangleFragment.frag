#version 460

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 color;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 cameraPosition;
layout (location = 3) in vec3 fragPos;


float roughness = 0.2;
float metlaness = 1;
float ao = 0.2;
vec3 albedo = vec3(0.0, 0.0, 0.7);

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

vec3 CalculateIrrandiance(){
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
    return Lo;
}

void main() {


    vec3 ambient = vec3(0.3) * albedo * ao;
    vec3 color = ambient + CalculateIrrandiance();

    outColor = vec4(color,1.0);
}