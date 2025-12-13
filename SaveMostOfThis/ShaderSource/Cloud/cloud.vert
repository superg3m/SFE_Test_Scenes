#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoord;

out vec2 v_TextureCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    v_TextureCoord = aTexCoord;
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1);
}