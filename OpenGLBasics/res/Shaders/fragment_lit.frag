#version 330
in vec2 uvCoord;
in vec3 vNormal;
in vec4 vPos;
in vec4 lightSpacePos;

// OUT VARIABLES
out vec4 color;

// UNIFORMS
uniform vec4 viewPos;
uniform sampler2D shadowMap;
uniform sampler2D diffuseTex;
uniform vec4 diffuseColor;

uniform struct DirLight
{
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    float energy;
} sun;

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
    //return (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
}

vec4 CalcDirLight(DirLight light, vec4 normal, vec4 viewDir)
{
    vec4 lightDir = normalize(light.direction);
    float shadow = ShadowCalulation(lightSpacePos, light.direction, normal);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // combine results
    vec4 ambient  = light.ambient;
    vec4 diffuse  = light.diffuse * diff * (light.energy * 0.1);
    if (length(diffuseColor.xyz) < 1.73)
    {
        ambient *= diffuseColor;
        diffuse *= diffuseColor;
    }
    else
    {
        diffuse *= texture(diffuseTex, uvCoord);
        ambient *= texture(diffuseTex, uvCoord);
    }
    // (ambient + (diffuse + specular) * shadow)
    return ambient + diffuse * (1 - shadow);
}

void main()
{
    vec4 viewDir = normalize(viewPos - vPos);
    vec4 normal = vec4(vNormal, 0);
    color = CalcDirLight(sun, normal, viewDir);

    // apply gamma correction
    float gamma = 1.25;
    color.rgb = pow(color.rgb, vec3(1.0/gamma));
}