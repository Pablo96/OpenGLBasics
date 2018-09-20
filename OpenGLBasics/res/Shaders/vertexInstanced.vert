#version 330
layout(location = 0)in vec3 position;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec2 texCoord;
layout(location = 3)in vec3 tangent;
layout(location = 4)in mat4 transform;
layout(location = 8)in mat4 model;
layout(location = 12)in mat3 normalMat;

out mat3 tbnMatrix;
out vec4 vPos;
out vec2 uvCoord;

void main()
{
    vec4 pos = vec4(position, 1.0);
    gl_Position = transform * pos;
    
    uvCoord = texCoord;
    vPos = model * pos;

    vec3 n = normalize(vec3(model *  vec4(normal, 0.0)));
    vec3 t = normalize(vec3(model *  vec4(tangent, 0.0)));
    t = normalize(t - dot(t, n) * n);
    vec3 b = cross(t, n);
    tbnMatrix = mat3(t, b, n);
}