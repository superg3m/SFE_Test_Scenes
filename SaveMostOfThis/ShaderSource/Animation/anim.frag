#version 330 core

struct Material {
    sampler2D diffuse_map;
    sampler2D specular_map;
    sampler2D emissive_map;

    vec3 ambient_color;
    vec3 diffuse_color;
    vec3 specular_color;

    
    float shininess;
    float opacity;
};

in vec2 v_TexCoord;
out vec4 FragColor;

uniform Material uMaterial;
uniform vec3 uLightPosition;
uniform bool uApplyEmissiveMaterial;

void main() {
    FragColor = vec4(texture(uMaterial.diffuse_map, v_TexCoord).rgb, 1);
}