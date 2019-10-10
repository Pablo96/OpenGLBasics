#version 430
layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 bloom_color;

uniform vec3 color;
uniform float intensity;

void main()
{
    vec3 color_emited = color * intensity;
    out_color = vec4(color_emited, 1);
    
    // BLOOM
    float brightness = out_color.r * 0.2126 +  out_color.g * 0.7152 + out_color.b * 0.0722;
    bloom_color = vec4(out_color.rgb * brightness, 1.0);
}