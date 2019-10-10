#version 430
layout(location = 0)in vec3 v_position;


uniform mat4 pv_mat;
uniform mat4 model;

void main()
{
    // World Position
    vec4 world_pos = model * vec4(v_position, 1.0);
    
    gl_Position = pv_mat * world_pos;
}