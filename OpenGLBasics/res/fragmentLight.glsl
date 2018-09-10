#version 330

// uniforms
uniform vec4 ourColor;
out vec4 color;

void main()
{
    color = ourColor;
}