#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec3 aTexCoord;
layout(location = 8) in mat4 uModel; // NOTE(Jovanni): Dynamic Instance Attribute

uniform mat4 uView;
uniform mat4 uProjection;

out vec2 v_TexCoord;

void main() {
    v_TexCoord = aTexCoord;
     
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1);
}