#version 330 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    vec4 bmp = texture(uTexture, v_TexCoord).rgba;
    if (bmp.a < 0.1) {
        discard;
    }

    FragColor = bmp;
}