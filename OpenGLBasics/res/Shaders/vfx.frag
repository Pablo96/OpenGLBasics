#version 330 core
out vec4 out_color;

in vec2 uv_coord;

uniform sampler2D screen;

void main()
{
    const float gamma = 1.0;
    const float exposure = 1.0;
    vec3 hdrColor = texture(screen, uv_coord).rgb;

    // reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

    out_color = vec4(mapped, 1);
}