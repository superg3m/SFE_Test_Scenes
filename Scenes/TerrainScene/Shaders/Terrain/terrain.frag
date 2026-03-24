#version 330 core

in vec3 v_FragPosition;
in vec2 v_TextureCoord;
in vec3 v_Normal;

out vec4 FragColor;

uniform vec3 uLightPosition;
uniform vec3 uCameraPosition;
uniform bool uRenderNormals;
uniform bool uRenderShading;

uniform sampler2D uHeightTexture;
uniform sampler2D uColorTexture;

float fresnel(float amount, vec3 normal, vec3 view) {
	return pow((1.0 - clamp(dot(normalize(normal), normalize(view)), 0.0, 1.0 )), amount);
}

void main() {
	vec3 N = normalize(v_Normal);
    vec3 L = normalize(uLightPosition - v_FragPosition);
    vec3 V = normalize(uCameraPosition - v_FragPosition);
    float lambertian_mask = max(dot(N, L), 0.0);
    
	vec3 texel_color = texture(uColorTexture, v_TextureCoord).rgb;

    vec3 ambient_light = (texel_color * 0.25);
    vec3 diffuse_light = (texel_color * lambertian_mask);

	if (uRenderNormals == true) {
        float f = fresnel(5.0, N, L);
        
        f = smoothstep(0.8, 1.0, f);

        // N += 1.0;
        // N /= 2.0;
        vec3 fresnel_color = (f * N) * 0.75; // vec3(1.0, 0.0, 0.0);
        FragColor = vec4((diffuse_light + ambient_light) + fresnel_color, 1.0);

        // FragColor = vec4(N, 1);
    } else if (uRenderShading == true) {
        FragColor = vec4((diffuse_light + ambient_light), 1.0);
    } else {
        FragColor = vec4(texel_color, 1.0);
    }
}