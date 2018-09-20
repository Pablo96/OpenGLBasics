#version 330
in vec2 uvCoord;
in vec4 vPos;
in mat3 tbnMatrix;

// OUT VARIABLES
out vec4 color;

// UNIFORMS
uniform vec4 viewPos;
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
} sun;

vec4 CalcDirLight(DirLight light, vec4 normal, vec4 viewDir)
{

    vec4 lightDir = normalize(light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec4 halfwayDir = normalize(viewDir - lightDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    //vec4 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec4 ambient  = light.ambient  * texture(material.diffuse, uvCoord);
    vec4 diffuse  = light.diffuse  * diff * texture(material.diffuse, uvCoord);
    vec4 specular = light.specular * spec * texture(material.specular, uvCoord).y;
    // (ambient + diffuse + specular) * AO
    return (ambient + diffuse + specular) * texture(material.specular, uvCoord).z;
}


void main()
{
    vec4 viewDir = normalize(viewPos - vPos);
    vec4 normal  = vec4(normalize(tbnMatrix * (255.0/128.0 * texture(material.normal, uvCoord).xyz - 1)), 0.0);
    
    color = CalcDirLight(sun, normal, viewDir);

    // apply gamma correction
    float gamma = 1.4;
    color.rgb = pow(color.rgb, vec3(1.0/gamma));
}