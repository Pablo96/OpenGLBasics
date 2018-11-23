#version 330
in vec4 vPos;
in vec2 uvCoord;
in vec3 vNormal;
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
    vec4 position;

    vec4 ambient;
    vec4 diffuse;
    float energy;
} sun;

struct TANGENTLIGHTINFO {
    vec3 pos; // only needed if no directional light
    vec3 viewPos;
    vec3 vPos;
} tan_light_info;


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = tbnMatrix * normalize(light.direction).xyz;
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    
    // combine results
    vec3 difTex = texture(material.diffuse, uvCoord).rgb;

    vec3 ambient  = light.ambient.xyz * difTex;
    vec3 diffuse  = light.diffuse.xyz * light.energy * diff * difTex * material.intensity;
    vec3 specular = vec3(0.2) * spec;

    return ambient + diffuse + specular;
}

void main()
{
    tan_light_info.pos = tbnMatrix * sun.position.xyz;

    // obtain normal from normal map in range [0,1]
    vec3 normal = texture(material.normal, uvCoord).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
    // View direction
    vec3 viewDir = normalize(tan_light_info.viewPos - tan_light_info.vPos);

    color = vec4(CalcDirLight(sun, normal, viewDir), 1.0);

    // apply gamma correction
    //float gamma = 1.25;
    //color.rgb = pow(color.rgb, vec3(1.0/gamma));
}