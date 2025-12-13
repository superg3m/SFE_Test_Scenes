#include <Shader/shader_cloud.hpp>

#include <rendering.hpp>

ShaderCloud::ShaderCloud(DS::Vector<const char*> shader_paths) {
    this->shader_paths = shader_paths;
    this->compile();
}

void ShaderCloud::compile() {
    this->program_id = this->createShaderProgram(this->shader_paths);
    
    this->uCloudTexture_Location = getUniformLocation("uCloudTexture", GL_SAMPLER_2D);
}

void ShaderCloud::setMaterial(const Material& material) const {}

void ShaderCloud::setCloudTexture(const Texture& cloud_texture) {
    this->use();
    glCheckError(glActiveTexture(GL_TEXTURE0 + 0));
    glCheckError(glBindTexture(GL_TEXTURE_2D, cloud_texture.id));
    glUniform1i(this->uCloudTexture_Location, 0);
}
