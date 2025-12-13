#version 330 core

in vec2 v_TextureCoord;

out vec4 FragColor;

uniform sampler2D uCloudTexture;
uniform float uOffsetX;
uniform float uOffsetY;

void main() {
    vec4 texel = texture(uCloudTexture, v_TextureCoord + vec2(uOffsetX, uOffsetY));
    texel.a = texel.r;

    FragColor = texel;
}