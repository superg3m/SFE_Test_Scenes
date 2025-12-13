#version 330 core

uniform sampler2D uTexture;

in vec2 v_TexCoord;
out vec4 FragColor;

void main() {
    FragColor = vec4(1, 0, 0, 1);
}