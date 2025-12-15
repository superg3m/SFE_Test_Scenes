#include "quad.hpp"

ShaderQuadInstance::ShaderQuadInstance(DS::Vector<const char*> shader_paths) {
    this->shader_paths = shader_paths;
    this->compile();
}

void ShaderQuadInstance::compile() {
    this->program_id = this->createShaderProgram(this->shader_paths);
}

void ShaderQuadInstance::setMaterial(const Material& material) const {}