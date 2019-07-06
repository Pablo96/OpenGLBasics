#version 430
out vec4 out_color;
in vec2 uv_coord;
in vec3 normal;
in vec3 in_position;
in mat3 in_tbn_mat;

uniform vec3 lightPos;
const vec3 light_color = vec3(1.0, 0.9, 0.8) * 2.5;
const float ao = 0.6;
const float PI = 3.14159265359;

uniform sampler2D diffuseMap;
uniform sampler2D normals_map;

float Fd_Lambert() {
    return 1.0 / PI;
}

void main()
{
    // obtain normal from normal map in range [0,1]
    vec3 normal = texture(normals_map, uv_coord).rgb;
    // transform normal vector to range [-1,1]
    normal = normal * 2.0 - 1.0;
    vec3 N = in_tbn_mat * normalize(normal);
    vec3 L = normalize(lightPos - in_position);
    
    // Shadow
    float NdotL = max(dot(N, L), 0.0);
    // Radiance
    vec3 color = texture(diffuseMap, uv_coord).rgb;
    vec3 Lo = color * light_color * NdotL * Fd_Lambert(); 

    // Ambient lieght
    vec3 ambient = vec3(0.03) * color * ao;
    // Final color
    vec3 obj_color = ambient + Lo;
    // HDR
    obj_color = obj_color / (obj_color + vec3(1.0f));
    // Gamma Correction
    obj_color = pow(obj_color, vec3(1.0/2.2));

    out_color = vec4(obj_color, 1);
}