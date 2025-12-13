#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 6) in ivec4 aBoneIDs;
layout(location = 7) in vec4 aWeights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uFinalBonesMatrices[MAX_BONES];

out vec2 v_TexCoord;

void main() {
    vec4 bone_space_position = vec4(0.0f);
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if (aBoneIDs[i] == -1) {
            continue;
        }

        if (aBoneIDs[i] >=MAX_BONES) {
            bone_space_position = vec4(aPos, 1);
            break;
        }

        vec4 local_position = uFinalBonesMatrices[aBoneIDs[i]] * vec4(aPos, 1);
        bone_space_position += local_position * aWeights[i];

        vec3 local_normal = mat3(uFinalBonesMatrices[aBoneIDs[i]]) * aNormal;
    }

    v_TexCoord = aTexCoord;

    gl_Position = projection * view * model * bone_space_position;
}