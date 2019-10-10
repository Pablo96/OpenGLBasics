#version 430
layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 bloom_color;


in vec2 uv_coord;
in vec3 normal;
in vec3 in_position;
in vec4 in_positionLightSpace;
in mat3 in_tbn_mat;

uniform vec3 lightPos;
const vec3 light_color = vec3(1.0, 0.9, 0.8) * 2.5;
const float ao = 0.6;
const float PI = 3.14159265359;

uniform sampler2D diffuseMap;
uniform sampler2D normals_map;
uniform sampler2D shadow_map;

float Fd_Lambert() {
    return 1.0 / PI;
}

float ShadowCalculation()
{
    // perform perspective divide
    vec3 projCoords = in_positionLightSpace.xyz / in_positionLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadow_map, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main()
{
    // obtain normal from normal map in range [0,1]
    vec3 normal = texture(normals_map, uv_coord).rgb;
    // transform normal vector to range [-1,1]
    normal = normal * 2.0 - 1.0;
    vec3 N = in_tbn_mat * normalize(normal);
    vec3 L = normalize(lightPos - in_position);
    
    float NdotL = max(dot(N, L), 0.0);
    // Radiance
    vec3 color = texture(diffuseMap, uv_coord).rgb;
    vec3 Lo =  color * light_color * NdotL * Fd_Lambert(); 

    // Ambient light
    vec3 ambient = vec3(0.03) * color * ao;
    
    // Shadow
    float shadow = ShadowCalculation();
    
    // Final color
    vec3 obj_color = ambient + (1.0 - shadow) * Lo;

    // check whether fragment output is higher than threshold, if so output as brightness color
    out_color = vec4(obj_color, 1);

    // BLOOM
    float brightness = out_color.r * 0.2126 +  out_color.g * 0.7152 + out_color.b * 0.0722;
    bloom_color = vec4(out_color.rgb * brightness, 1.0);
}