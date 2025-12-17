#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoord;
layout(location = 8) in vec3 aColor;
layout(location = 9) in mat4 uModel;

uniform mat4 uView;
uniform mat4 uProjection;

out vec2 v_TexCoord;
out vec3 v_Color;

void main() {
    v_Color = aColor;
    v_TexCoord = aTexCoord;
    gl_Position = uProjection * uView * transpose(uModel) * vec4(aPos, 1);
}