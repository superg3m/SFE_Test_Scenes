#version 330 core

in vec3 v_FragPosition;
in vec2 v_TextureCoord;
in vec3 v_Normal;

out vec4 FragColor;

uniform vec3 uLightPosition;
uniform bool uRenderNormals;
uniform bool uRenderShading;

uniform sampler2D uHeightTexture;
uniform sampler2D uColorTexture;

void main() {
	vec3 N = normalize(v_Normal);
    vec3 L = normalize(uLightPosition - v_FragPosition);
    float lambertian_mask = max(dot(N, L), 0.0);
	vec4 texel_color = texture(uColorTexture, v_TextureCoord);

    vec4 ambient_light = (texel_color * 0.25);
    vec4 diffuse_light = (texel_color * lambertian_mask);

	if (uRenderNormals == true) {
        N += 1.0;
        N /= 2.0;
        FragColor = vec4(N, 1);
    } else if (uRenderShading == true) {
        FragColor = diffuse_light + ambient_light;
    } else {
        FragColor = texel_color;
    }
}