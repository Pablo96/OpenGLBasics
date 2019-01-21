#version 330
layout(location = 0)in vec3 position;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec2 texCoord;
layout(location = 4)in mat4 transform;
layout(location = 8)in mat4 model;
layout(location = 12)in uvec4 indices; //unisgned int vector 4
layout(location = 13)in vec4 weights;

const int MAX_BONES = 50;

uniform mat4 lightSpaceMatrix;
uniform mat4 bones[MAX_BONES];

out vec2 uvCoord;
out vec3 vNormal;
out vec4 vPos;
out vec4 lightSpacePos;

void main()
{
    mat4 boneTransform =  bones[indices.x] * weights.x;
         boneTransform += bones[indices.y] * weights.y;
         boneTransform += bones[indices.z] * weights.z;
         boneTransform += bones[indices.w] * weights.w;
    
    vec4 pos = boneTransform * vec4(position, 1.0);
    gl_Position = transform * pos;
    
    uvCoord = texCoord;
    vNormal = normal;
    vPos = model * pos;

    lightSpacePos =  lightSpaceMatrix * vPos;
}