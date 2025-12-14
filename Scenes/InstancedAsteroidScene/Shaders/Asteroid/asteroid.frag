#version 330 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D diffuse;

void main() {
    FragColor = texture(diffuse, v_TexCoord);
}

