#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 aInstanceMatrix;

out vec2 v_TexCoord;

uniform mat4 uProjection;
uniform mat4 uView;

void main() {
    TexCoords = aTexCoord;
    gl_Position = uProjection * uView * aInstanceMatrix * vec4(aPos, 1.0); 
}