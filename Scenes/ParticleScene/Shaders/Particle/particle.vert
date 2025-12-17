#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoord;
layout(location = 8) in vec3 aCenter;
layout(location = 9) in vec3 aColor;

uniform mat4 uView;
uniform mat4 uProjection;

out vec2 v_TexCoord;
out vec3 v_Color;

void main() {
    vec2 BillboardSize = vec2(1); 
    vec3 CameraRight_worldspace = {uView[0][0], uView[1][0], uView[2][0]};
    vec3 CameraUp_worldspace = {uView[0][1], uView[1][1], uView[2][1]};

    vec3 vertexPosition_worldspace = (
        aCenter + 
        (CameraRight_worldspace * aPos.x * BillboardSize.x) + 
        (CameraUp_worldspace * aPos.y * BillboardSize.y)
    );

    v_Color = aColor;
    v_TexCoord = aTexCoord;
    gl_Position = uProjection * uView * vec4(vertexPosition_worldspace, 1);
}