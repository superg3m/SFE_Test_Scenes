#version 330 core

in vec2 v_TexCoord;
in vec3 v_Color;
in vec3 v_FragPos;
in vec3 v_Normal;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec3 uViewPosition;

float fresnel(float amount, vec3 normal, vec3 view) {
	return pow((1.0 - clamp(dot(normalize(normal), normalize(view)), 0.0, 1.0 )), amount);
}

void main() {
    vec3 view_direction = normalize(uViewPosition - v_FragPos);
    vec3 normal = normalize(v_Normal);

    vec3 texel = texture(uTexture, v_TexCoord).rgb;
    // float alpha = 1 - distance(v_TexCoord, vec2(0.5));
    // alpha = smoothstep(0.5, 0.8, alpha);
    // float f_mask = 1.0 - fresnel(3.0, normal, view_direction);
    //FragColor = vec4(texel * v_Color + f_mask, alpha);
    
    FragColor = vec4(texel * v_Color, 1.0);
}