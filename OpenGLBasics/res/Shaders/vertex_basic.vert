#version 330
layout(location = 0)in vec3 position;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec2 texCoord;

out vec2 uvCoord;
out vec4 vPos;
out vec4 vNormal;

uniform mat4 transform;
uniform mat4 model;

void main()
{
    vec4 pos = vec4(position, 1.0);
    gl_Position = transform * pos;
    uvCoord = texCoord;
    vNormal = model * vec4(normal, 0);
    vPos = model * pos;
}