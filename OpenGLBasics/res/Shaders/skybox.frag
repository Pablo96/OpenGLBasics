#version 330 core
out vec4 out_color;

in vec3 uv_coord;

uniform samplerCube diffuse;

void main()
{    
    out_color = texture(diffuse, uv_coord);
}