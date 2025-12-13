#version 330 core

struct Material {
    sampler2D diffuse_map;
    bool has_diffuse_map;

    sampler2D specular_map;
    bool has_specular_map;

    sampler2D emissive_map;
    bool has_emissive_map;

    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;

    float shininess;
    float opacity;

    bool has_normals;
    bool has_texcoord;
};

in vec3 v_FragPosition;
in vec3 v_Normal;
in vec2 v_TexCoord;
out vec4 FragColor;

uniform vec3 uLightPosition;
uniform vec3 uCameraPosition;
uniform Material uMaterial;
uniform bool uApplyEmissiveMaterial;

void main() {
    vec3 final_rgb = vec3(0.0);

    vec3 base_color;
    if (uMaterial.has_diffuse_map && uMaterial.has_texcoord) {
        base_color = texture(uMaterial.diffuse_map, v_TexCoord).rgb;
    } else {
        base_color = uMaterial.diffuse_color;
    }

    vec3 ambient_light = (base_color * 1.0); // 0.25
    final_rgb += ambient_light;

    if (uMaterial.has_normals || !uMaterial.has_normals) {
        vec3 N = normalize(v_Normal);
        vec3 L = normalize(uLightPosition - v_FragPosition);
        float lambertian_mask = max(dot(N, L), 0.0);
        vec3 diffuse_light = (base_color * lambertian_mask);

        final_rgb += diffuse_light;

        /*
        if ((uMaterial.has_specular_map && uMaterial.has_texcoord) || uMaterial.shininess > 0.0) {
            vec3 V = normalize(uCameraPosition - v_FragPosition);
            vec3 H = normalize(L + V);
            float specular_mask = pow(max(dot(N, H), 0.0), uMaterial.shininess);
            vec3 specular_map_color = uMaterial.has_specular_map ? texture(uMaterial.specular_map, v_TexCoord).rgb : uMaterial.specular_color;
            vec3 specular_light = specular_map_color * specular_mask;

            final_rgb += specular_light;
        }
        */
    }

    if (uApplyEmissiveMaterial && uMaterial.has_emissive_map && uMaterial.has_texcoord) {
        vec3 emissive = texture(uMaterial.emissive_map, v_TexCoord).rgb;
        final_rgb += emissive;
    }

    FragColor = vec4(final_rgb, uMaterial.opacity);
}