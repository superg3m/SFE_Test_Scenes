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
    vec2 billboard_size = vec2(1.5); 

    vec3 worldspace_camera_right_vector = vec3(uView[0][0], uView[1][0], uView[2][0]);
    vec3 worldspace_camera_up_vector = vec3(uView[0][1], uView[1][1], uView[2][1]);

    vec3 worldspace_cannoncial_vertex_position = (
        aCenter + 
        (worldspace_camera_right_vector * aPos.x * billboard_size.x) + 
        (worldspace_camera_up_vector * aPos.y * billboard_size.y)
    );

    v_Color = aColor;
    v_TexCoord = aTexCoord;
    gl_Position = uProjection * uView * vec4(worldspace_cannoncial_vertex_position, 1);
}