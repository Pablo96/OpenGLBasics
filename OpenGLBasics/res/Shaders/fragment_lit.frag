#version 330
in vec2 uvCoord;
in vec4 vNormal;
in vec4 vPos;

// OUT VARIABLES
out vec4 color;

uniform sampler2D diffuse;

void main()
{
    vec4 light_dir = normalize(vec4(-1, -1, -1, 0));
    vec4 normal = normalize(vNormal);
    float dif = dot(normal, -light_dir);
    color = texture2D(diffuse, uvCoord);
    vec4 ambient = color * 0.2;
    color = color * dif + ambient;
}