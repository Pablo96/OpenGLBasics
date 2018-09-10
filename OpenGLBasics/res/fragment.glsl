#version 330
// IN VARIABLES
in vec2 uvCoord;


// UNIFORMS
uniform sampler2D diffuse;


// OUT VARIABLES
out vec4 color;


void main()
{
    color = texture(diffuse, uvCoord);
}