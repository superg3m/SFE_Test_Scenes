#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 v_FragPosition;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main() {
    v_FragPosition = vec3(uModel * vec4(aPos, 1.0f));
    v_Normal = mat3(transpose(inverse(uModel))) * aNormal;
    v_TexCoord = aTexCoord;

    gl_Position = uProjection * uView * vec4(v_FragPosition, 1.0f);
}