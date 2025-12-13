#version 330 core

in vec3 TexCoord;
uniform samplerCube uSkyboxTexture;

out vec4 FragColor;

void main() {    
    FragColor = texture(uSkyboxTexture, TexCoord);
}