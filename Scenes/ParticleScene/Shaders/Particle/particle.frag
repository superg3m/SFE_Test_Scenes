#version 330 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    FragColor = vec4(texture(uTexture, v_TexCoord).rgb, 1);
}