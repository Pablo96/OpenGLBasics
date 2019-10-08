#version 330 core
layout (location = 0) in vec3 v_position;

out vec3 uv_coord;

uniform mat4 pv_transform;

void main()
{
    uv_coord = v_position;
    vec4 pos = pv_transform * vec4(v_position, 1);
    // To avoid z faighting with the clear color
    // we need to add a tiny ammount to the w component
    // but no so tiny to give precision problems
    gl_Position = vec4(pos.xyz, pos.z + 0.0001);
} 