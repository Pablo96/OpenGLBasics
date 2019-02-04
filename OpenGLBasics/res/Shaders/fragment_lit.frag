#version 330
in vec2 uvCoord;
in vec3 vNormal;
in vec4 vPos;
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

vec4 CalcDirLight(DirLight light, vec4 normal, vec4 viewDir)
{
    vec4 lightDir = normalize(light.direction);
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
    return ambient + diffuse;
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