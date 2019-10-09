#version 330 core
layout (location = 0) in vec3 v_position;

uniform mat4 lightspace_mat;
uniform mat4 model;

void main()
{
    gl_Position = lightspace_mat * model * vec4(v_position, 1);
}