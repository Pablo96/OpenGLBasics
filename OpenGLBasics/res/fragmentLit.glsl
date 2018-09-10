#version 330
#define NR_POINT_LIGHTS 4
// IN VARIABLES
in vec2 uvCoord;
in vec4 vPos;
in vec3 vNormal;

// Object Types
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct PointLight {
    vec4 position;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirLight
{
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
};

// UNIFORMS
uniform Material material;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform DirLight sun;
uniform vec4 viewPos;

// OUT VARIABLES
out vec4 color;

// GLOBALS
vec4 norm = normalize(vec4(vNormal, 0.0));
float near = 0.1; 
float far  = 15.0;

// FUNCTIONS
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

vec4 CalcPointLight(PointLight light, vec4 normal, vec4 fragPos, vec4 viewDir)
{
    vec4 lightDir = normalize(light.position - vPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec4 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance    = length(light.position - vPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec4 ambient  = light.ambient  * texture(material.diffuse, uvCoord);
    vec4 diffuse  = light.diffuse  * diff * texture(material.diffuse, uvCoord);
    vec4 specular = light.specular * spec * texture(material.specular, uvCoord);
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec4 CalcDirLight(DirLight light, vec4 normal, vec4 viewDir)
{
    vec4 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec4 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec4 ambient  = light.ambient  * texture(material.diffuse, uvCoord);
    vec4 diffuse  = light.diffuse  * diff * texture(material.diffuse, uvCoord);
    vec4 specular = light.specular * spec * (1 - texture(material.specular, uvCoord).y);
    return (ambient + diffuse + specular);
}


void main()
{
    // properties
    vec4 norm = normalize(vec4(vNormal, 0.0));
    vec4 viewDir = normalize(viewPos - vPos);

    // phase 1: Directional lighting
    vec4 result = CalcDirLight(sun, norm, viewDir);
    // phase 2: Point lights
    /*
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, vPos, viewDir); 
    */
    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    color = vec4(vec3(1-depth), 1.0) * result;
}