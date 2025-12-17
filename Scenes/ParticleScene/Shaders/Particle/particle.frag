#version 330 core

in vec2 v_TexCoord;
in vec3 v_Color;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    FragColor = vec4(texture(uTexture, v_TexCoord).rgb * v_Color, 1);
}