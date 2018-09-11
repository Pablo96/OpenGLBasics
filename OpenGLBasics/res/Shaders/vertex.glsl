#version 330
layout(location = 0)in vec3 position;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec2 texCoord;

out vec2 uvCoord;
out vec4 vPos;
out vec3 vNormal;

uniform mat4 transform;
uniform mat4 model;
uniform mat3 normalMat;

void main()
{
    vec4 pos = vec4(position, 1.0);
    gl_Position = transform * pos;
    uvCoord = texCoord;
    vNormal = normalMat *  normal;
    vPos = model * pos;
}