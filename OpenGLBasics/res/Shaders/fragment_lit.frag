#version 330
in vec2 uvCoord;
in vec4 vNormal;
in vec4 vPos;
in vec4 lightSpacePos;
in mat3 tbnMatrix;

// OUT VARIABLES
out vec4 color;

// UNIFORMS
uniform vec4 viewPos;
uniform sampler2D shadowMap;
uniform struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    float shininess;
} material;

uniform struct DirLight
{
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
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
    // specular shading
    //vec4 halfwayDir = normalize(viewDir - lightDir);
    //float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec4 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec4 ambient  = light.ambient  * texture(material.diffuse, uvCoord);
    vec4 diffuse  = light.diffuse  * diff * texture(material.diffuse, uvCoord) * (light.energy * 0.1);
    vec4 specular = light.specular * spec * texture(material.specular, uvCoord).y;
    float AO = texture(material.specular, uvCoord).z;
    // (ambient + (diffuse + specular) * shadow) * AO
    return (ambient + (diffuse + specular) * (1 - shadow)) * AO;
}

void main()
{
    vec4 viewDir = normalize(viewPos - vPos);
    //vec4 normal  = vec4(normalize(tbnMatrix * (255.0/128.0 * texture(material.normal, uvCoord).xyz - 1)), 0.0);
    vec4 normal = vNormal;
    color = CalcDirLight(sun, normal, viewDir);

    // apply gamma correction
    float gamma = 1.25;
    color.rgb = pow(color.rgb, vec3(1.0/gamma));
}