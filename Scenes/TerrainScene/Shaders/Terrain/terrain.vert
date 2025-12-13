#version 330 core

layout(location = 0) in vec3 aPos;       // x,z in [0,width]x[0,height]
layout(location = 1) in vec3 aNormal;    // not used (heightmap overrides)
layout(location = 2) in vec2 aTexCoord;  // UV in [0,1]

out vec3 v_FragPosition;
out vec2 v_TextureCoord;
out vec3 v_Normal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

uniform float uWidth;
uniform float uHeight;
uniform int uHeightBoost;
uniform sampler2D uHeightTexture;

void main()
{
    float X_STEP = 1.0 / uWidth;
    float Z_STEP = 1.0 / uHeight;

    vec2 tc_uv = aTexCoord;
    float hC = texture(uHeightTexture, tc_uv).r * uHeightBoost;
    vec3 worldPos = aPos;
    worldPos.y += hC;

    float hL = texture(uHeightTexture, tc_uv - vec2(X_STEP, 0.0)).r * uHeightBoost;
    float hR = texture(uHeightTexture, tc_uv + vec2(X_STEP, 0.0)).r * uHeightBoost;
    float hD = texture(uHeightTexture, tc_uv - vec2(0.0, Z_STEP)).r * uHeightBoost;
    float hU = texture(uHeightTexture, tc_uv + vec2(0.0, Z_STEP)).r * uHeightBoost;

    vec3 approximate_normal = normalize(vec3(hL - hR, 2.0, hD - hU));
    mat3 normal_matrix = transpose(inverse(mat3(uModel)));

    v_FragPosition = vec3(uModel * vec4(worldPos, 1.0));
    v_TextureCoord = tc_uv;
    v_Normal = normal_matrix * approximate_normal;

    gl_Position = uProjection * uView * vec4(v_FragPosition, 1.0);
}
