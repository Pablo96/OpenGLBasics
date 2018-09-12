#version 330
layout(location = 0)in vec3 position;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec2 texCoord;
layout(location = 3)in mat4 transform;
layout(location = 7)in mat4 model;
layout(location = 11)in mat3 normalMat;

out vec4 vPos;
out vec3 vNormal;
out vec2 uvCoord;

void main()
{
    vec4 pos = vec4(position, 1.0);
    gl_Position = transform * pos;
    uvCoord = texCoord;
    vNormal = normalMat *  normal;
    vPos = model * pos;
}