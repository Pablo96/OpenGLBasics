#version 330 core
out vec4 out_color;

in vec2 uv_coord;

uniform sampler2D screen;
uniform sampler2D bloomScreen;

void main()
{
    const float gamma = 1.2;
    const float exposure = 2.0;
    vec3 hdrColor = texture(screen, uv_coord).rgb;
    vec3 bloomColor = texture(bloomScreen, uv_coord).rgb;
    hdrColor += bloomColor * 2; // additive blending
    
    // reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

    out_color = vec4(mapped, 1);
}