// Vibeout (https://github.com/smogpill/vibeout)
// SPDX-FileCopyrightText: 2025 Jounayd ID SALAH
// SPDX-License-Identifier: MIT

// Refs:
// - https://github.com/gamehacker1999/DX12Engine/blob/master/Shaders/Lighting.hlsli
// - https://learnopengl.com/PBR/Lighting
// - https://www.shadertoy.com/view/3tycWd
// - https://gamedev.stackexchange.com/questions/108856/fast-position-reconstruction-from-depth/111885#111885
// - https://github.com/Angelo1211/HybridRenderingEngine/blob/master/assets/shaders/PBRClusteredShader.frag
// - http://www.codinglabs.net/article_physically_based_rendering_cook_torrance.aspx
const float PI = 3.14159265359;

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 ComputeF0(vec3 albedo, float metallic)
{
    // Correcting zero incidence reflection
    vec3 F0 = vec3(0.04); 
    return mix(F0, albedo, metallic);
}

vec3 Reflectance(vec3 fragPos, vec3 fragNormal, vec3 lightPos, vec3 light, vec3 viewDir, vec3 albedo, float roughness, float metallic, vec3 F0)
{
    vec3 N = fragNormal;
    vec3 V = viewDir;

    // Calculate per-light radiance
    vec3 L = normalize(lightPos - fragPos);
    vec3 H = normalize(V + L);
    float distance = max(0.1f, length(lightPos - fragPos));
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light * attenuation;
        
    // Cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
        
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
            
    // Add to outgoing radiance Lo
    float NdotL = max(0.0, dot(N, L));
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

// Attempt to remove the view dependent light to help indirect lighting. Not sure if that's the right strategy
vec3 ReflectanceForIndirectLighting(vec3 fragPos, vec3 fragNormal, vec3 lightPos, vec3 light, float metallic)
{
    vec3 N = fragNormal;

    // Calculate per-light radiance
    vec3 L = normalize(lightPos - fragPos);
    float distance = max(0.1f, length(lightPos - fragPos));

    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light * attenuation;
    vec3 kD = vec3(1.0 - metallic);

    // Add to outgoing radiance Lo
    float NdotL = max(0.0, dot(N, L));
    return kD / PI * radiance * NdotL;
}

/*
float4 PixelShaderFunction(VertexShaderOutput input) : SV_TARGET0
{
    ...

    float3 surface = tex2D(surfaceMap_Sampler, input.Texcoord).rgb;   
    float ior = 1 + surface.r;
    float roughness = saturate(surface.g - EPSILON) + EPSILON;
    float metallic = surface.b;

    // Calculate colour at normal incidence
    float3 F0 = abs ((1.0 - ior) / (1.0 + ior));
    F0 = F0 * F0;
    F0 = lerp(F0, materialColour.rgb, metallic);
        
    // Calculate the specular contribution
    float3 ks = 0;
    float3 specular = GGX_Specular(specularCubemap, normal, viewVector, roughness, F0, ks );
    float3 kd = (1 - ks) * (1 - metallic);

    // Calculate the diffuse contribution
    float3 irradiance = texCUBE(diffuseCubemap_Sampler, normal ).rgb;
    float3 diffuse = materialColour * irradiance;

    return float4( kd * diffuse + ks * specular, 1);     
}
*/