#version 330
in vec2 uvCoord;
in vec3 vNormal;
in vec4 vPos;

// OUT VARIABLES
out vec4 color;

// UNIFORMS
uniform sampler2D diffuse;





void main()
{
    vec3 lightDir = vec3(-1, -1, -1);
    vec4 lightAmbient = vec4(0.2, 0.2, 0.2, 1.0);
    vec4 lightDiffuse = vec4(1.0, 0.7, 0.0, 1.0);
    vec4 diffuseUnlit = texture(diffuse, uvCoord);
    
    // light calc
    vec4 ambientLight = diffuseUnlit * lightAmbient;
    // diffuse shading
    float diff = max(dot(-lightDir, vNormal), 0.0);
    vec4 diffuseLight = diffuseUnlit * diff * lightDiffuse;
    color = ambientLight + diffuseLight;
}