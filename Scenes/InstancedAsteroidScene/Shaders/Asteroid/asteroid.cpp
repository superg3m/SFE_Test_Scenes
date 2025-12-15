#include "asteroid.hpp"

ShaderAsteroid::ShaderAsteroid(DS::Vector<const char*> shader_paths) {
    this->shader_paths = shader_paths;
    this->compile();
}

void ShaderAsteroid::compile() {
    this->program_id = this->createShaderProgram(this->shader_paths);
}

void ShaderAsteroid::setMaterial(const Material& material) const {}

void ShaderAsteroid::setTexture(Texture &texture) {
    this->use();
    glCheckError(glActiveTexture(GL_TEXTURE0 + 0));
    glCheckError(glBindTexture(GL_TEXTURE_2D, texture.id));
    glUniform1i(this->uTexture_Location, 0);
}