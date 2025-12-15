#include "asteroid.hpp"

ShaderAsteroid::ShaderQuadInstance(DS::Vector<const char*> shader_paths) {
    this->shader_paths = shader_paths;
    this->compile();
}

void ShaderAsteroid::compile() {
    this->program_id = this->createShaderProgram(this->shader_paths);
}

void ShaderAsteroid::setMaterial(const Material& material) const {}