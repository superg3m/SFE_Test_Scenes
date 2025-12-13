#include <Shader/shader_particle.hpp>

ShaderParticle::ShaderParticle(DS::Vector<const char*> shader_paths) {
    this->shader_paths = shader_paths;
    this->compile();
}

void ShaderParticle::compile() {
    this->program_id = this->createShaderProgram(this->shader_paths);
    
    this->uTexture_Location = this->getUniformLocation("uTexture", GL_SAMPLER_2D);
}

void ShaderParticle::setMaterial(const Material& material) const {}

void ShaderParticle::setTexture(Texture texture) const {
    this->use();

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    this->setInt(this->uTexture_Location, 0); 
}