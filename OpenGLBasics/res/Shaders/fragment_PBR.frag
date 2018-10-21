#version 330
in vec2 uvCoord;
in vec4 vNormal;
in vec4 vPos;
in vec4 lightSpacePos;
in mat3 tbnMatrix;

// OUT VARIABLES
out vec4 FragColor;

// UNIFORMS
uniform vec4 viewPos;
uniform sampler2D shadowMap;
uniform struct Material
{
    sampler2D albedo;
    sampler2D MRA; //metallic Roughness AmbientOcclusion
} material;

uniform struct DirLight
{
    vec4 direction;
    vec4 position;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float energy;
} sun;


const float PI = 3.14159265359;


float ShadowCalulation(vec4 posLightSpace, vec4 lightDir, vec4 normal)
{
    // Normalize to device coordinates
    // perform perspective divide (similar to that thing we do when divide X and Y by Z)
    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    // since textures are in range [0.0, 1.0] we convert NDC to same range
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    if(currentDepth > 1.0)
        return 0.0;
    
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    return shadow;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
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

vec3 CalcDirLight(DirLight light, vec3 N, vec3 V)
{
    // calculate per-light radiance
    vec3 L = normalize(light.position - vPos).xyz;
    vec3 H = normalize(V + L);
    float distance    = length(light.position - vPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance     = light.diffuse.rgb * attenuation * light.energy;        
    
    // cook-torrance brdf
    vec3 albedo = texture(material.albedo, uvCoord).rgb;
    float metallic = texture(material.MRA, uvCoord).r;
    float roughness = texture(material.MRA, uvCoord).g;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    float NDF = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular     = numerator / max(denominator, 0.001);
    
    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);                
    return (kD * albedo / PI + specular) * radiance * NdotL; 
}

void main()
{
    vec4 V = normalize(viewPos - vPos);
    vec4 N = normalize(vNormal);
    vec3 color = CalcDirLight(sun, N.xyz, V.xyz);
    
    vec3 albedo = texture(material.albedo, uvCoord).rgb;
    float AO = texture(material.MRA, uvCoord).b;
    vec3 ambient = vec3(0.05) * albedo * AO;
    float shadow = ShadowCalulation(lightSpacePos, sun.direction, N);
    color = ambient + color * (1 - shadow);
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    FragColor = vec4(color, 1.0);
}