#version 430
layout(location = 0)in vec3 v_position;
layout(location = 1)in vec3 v_normal;
layout(location = 2)in vec2 v_uvCoord;
layout(location = 3)in vec4 v_tangent;
layout(location = 4)in vec4 v_bitangent;

out vec3 normal;
out vec2 uv_coord;
out vec3 in_position;
out vec4 in_positionLightSpace;
out mat3 in_tbn_mat;

uniform mat4 pv_mat;
uniform mat4 lightspace_mat;
uniform mat4 model;

void main()
{
    uv_coord = v_uvCoord;
    normal = mat3(model) * v_normal;

    // World Position
    vec4 world_pos = model * vec4(v_position, 1.0);
    in_position = world_pos.xyz;
    // Light Space position
    in_positionLightSpace = lightspace_mat * world_pos;
    
    gl_Position = pv_mat * world_pos;

    vec3 N = normalize(vec3(model * vec4(v_normal, 0.0)));
    vec3 T = normalize(v_tangent.w * vec3(model * vec4(v_tangent.xyz,   0.0)));
    vec3 B = cross(T, N);
    in_tbn_mat = mat3(T, -B, N);
}