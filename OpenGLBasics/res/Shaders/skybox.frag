#version 330 core
layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 bloom_color;

in vec3 uv_coord;

uniform samplerCube diffuse;

void main()
{    
    out_color = texture(diffuse, uv_coord);

    // BLOOM
    float brightness = out_color.r * 0.2126 +  out_color.g * 0.7152 + out_color.b * 0.0722;
    bloom_color = vec4(out_color.rgb * brightness, 1.0);
}