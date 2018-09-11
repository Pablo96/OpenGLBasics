#version 330
layout(location = 0)in vec3 position;
layout(location = 1)in vec3 normal;
layout(location = 2)in vec2 texCoord;


uniform mat4 transforms[256];

out vec2 uvCoord;

void main()
{
    vec4 pos = vec4(position, 1.0);
    gl_Position = transforms[gl_InstanceID] * pos;
    uvCoord = texCoord;
}