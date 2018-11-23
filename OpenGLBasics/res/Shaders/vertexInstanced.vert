#version 330
layout(location = 0)in vec3 position;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec2 texCoord;
layout(location = 3)in vec3 tangent;
layout(location = 4)in mat4 transform;
layout(location = 8)in mat4 model;

out vec4 vPos;
out vec2 uvCoord;
out vec3 vNormal;
out mat3 tbnMatrix;


void main()
{
    vec4 pos = vec4(position, 1.0);
    gl_Position = transform * pos;
    
    uvCoord = texCoord;
    vPos = model * pos;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vNormal = normalize(normalMatrix * normal);
    
    vec3 n = vNormal.xyz;
    vec3 t = normalize(normalMatrix * tangent);
    t = normalize(t - dot(t, n) * n);
    vec3 b = cross(n, t);
    
    tbnMatrix = transpose(mat3(t, b, n));
}