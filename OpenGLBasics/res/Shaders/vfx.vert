#version 330 core
layout (location = 0) in vec3 v_position;

out vec2 uv_coord;

void main()
{
    uv_coord = (v_position.xy + vec2(1, 1)) / 2.0f;
    gl_Position = vec4(v_position.xy, 0, 1);
}