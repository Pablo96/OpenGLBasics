#version 330
layout(location = 0)in vec3 position;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec2 texCoord;
layout(location = 4)in mat4 transform;
layout(location = 8)in mat4 model;
layout(location = 12)in uvec4 indices; //unisgned int vector 4
layout(location = 13)in vec4 weights;

uniform mat4 lightSpaceMatrix;

out vec2 uvCoord;
out vec3 vNormal;
out vec4 vPos;
out vec4 lightSpacePos;

void main()
{
    vec4 pos = vec4(position, 1.0);
    gl_Position = transform * pos;
    
    uvCoord = texCoord;
    vNormal = normal;
    vPos = model * pos;

    lightSpacePos =  lightSpaceMatrix * vPos;
}