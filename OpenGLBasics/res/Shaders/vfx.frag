#version 330 core
out vec4 out_color;

in vec2 uv_coord;

uniform sampler2D screen;

void main()
{
    vec4 color = texture(screen, uv_coord);
}