#version 330
// IN VARIABLES
in vec2 uvCoord;


// UNIFORMS
uniform vec4 diffuseColor;
uniform sampler2D diffuse;


// OUT VARIABLES
out vec4 color;


void main()
{
    if (length(diffuseColor) < 1.73)
        color = diffuseColor;
    else
        color = texture(diffuse, uvCoord);
}