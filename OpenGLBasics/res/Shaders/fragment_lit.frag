#version 330
in vec2 uvCoord;
in vec4 vNormal;
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
    float intensity;
    float shininess;
} material;

uniform struct DirLight
{
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    float energy;
} sun;


vec4 CalcDirLight(DirLight light, vec4 normal, vec4 viewDir)
{
    vec4 lightDir = normalize(light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // combine results
    vec4 difTex = texture(material.diffuse, uvCoord);
    vec4 ambient  = light.ambient * difTex;
    vec4 diffuse  = light.diffuse * light.energy * diff * difTex * material.intensity;
    return ambient + diffuse;
}

void main()
{
    vec4 viewDir = normalize(viewPos - vPos);
    vec3 normal = 2 * texture(material.normal, uvCoord).xyz - 1;
    normal = tbnMatrix * normal;
    normal =  normalize(normal);
    //vec4 normal  = vec4(normalize(tbnMatrix * (255.0/128.0 * texture(material.normal, uvCoord).xyz - 1)), 0.0);
    //vec4 normal = normalize(vNormal);
    color = CalcDirLight(sun, vec4(normal, 0.0), viewDir);

    // apply gamma correction
    //float gamma = 1.25;
    //color.rgb = pow(color.rgb, vec3(1.0/gamma));
}