#version 330
layout(location = 0)in vec3 position;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec2 texCoord;
layout(location = 3)in vec3 tangent;
layout(location = 4)in mat4 transform;
layout(location = 8)in mat4 model;

out vec4 vPos;
out vec2 uvCoord;
out vec4 vNormal;
out mat3 tbnMatrix;

void main()
{
    vec4 pos = vec4(position, 1.0);
    gl_Position = transform * pos;
    
    uvCoord = texCoord;
    vPos = model * pos;
    vNormal = model * vec4(normal, 0.0);

    // World space normal
    vec3 n = normalize(model *  vec4(normal, 0.0)).xyz;
    // World space tangent
    vec3 t = normalize(model * vec4(tangent, 0.0)).xyz;
    t = normalize(t - dot(t, n) * n);
    // world space bitangent
    vec3 b = cross(t, n);
    
    tbnMatrix = mat3(t, b, n);
}