#version 330
layout(location = 0)in vec3 position;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec2 texCoord;
layout(location = 3)in vec4 weights;
layout(location = 4)in uvec4 indices; //unisgned int vector 4
layout(location = 5)in mat4 transform;
layout(location = 9)in mat4 model;

const int MAX_BONES = 50;

uniform vec4 colorNormal = vec4(0,0,0,1);
uniform mat4 bones[MAX_BONES];

out vec2 uvCoord;
out vec3 vNormal;
out vec4 vPos;

void main()
{
    // the identity matrix is there for those who has not weights (should be optimized so only mehses with at least one weight use this shader)
    mat4 boneTransform =  mat4(1.0) + bones[indices.x] * weights.x + bones[indices.y] * weights.y
                       +  bones[indices.z] * weights.z + bones[indices.w] * weights.w;


    vec4 pos = boneTransform * vec4(position, 1.0);
    gl_Position = transform * pos;
    
    uvCoord = texCoord;
    vNormal = normal;
    vPos = model * pos;
}