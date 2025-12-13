#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 8) in vec2 aOffset;

void main() {
    vec2 pos = aPos.xy * (gl_InstanceID / 100.0);
    gl_Position = vec4(pos + aOffset, 0, 1.0);
}