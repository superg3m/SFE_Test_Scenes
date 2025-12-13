#include <Shader/shader_stencil_outline.hpp>

ShaderStencilOutline::ShaderStencilOutline(DS::Vector<const char*> shader_paths) {
    this->shader_paths = shader_paths;
    this->program_id = this->createShaderProgram(shader_paths);
}

void ShaderStencilOutline::compile() {
    this->program_id = this->createShaderProgram(this->shader_paths);

    this->uOutlineScale_Location = this->getUniformLocation("uOutlineScale", GL_FLOAT);
}

void ShaderStencilOutline::setMaterial(const Material& material) const {}

void ShaderStencilOutline::setOutlineScale(float s) const {
    this->use();
    this->setFloat(this->uOutlineScale_Location, s); 
}