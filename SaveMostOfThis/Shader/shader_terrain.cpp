#include <Shader/shader_terrain.hpp>
#include <renderer.hpp>

ShaderTerrain::ShaderTerrain(DS::Vector<const char*> shader_paths) {
    this->shader_paths = shader_paths;
    this->compile();
}

void ShaderTerrain::compile() {
    this->program_id = this->createShaderProgram(this->shader_paths);

    this->uWidth_Location = this->getUniformLocation("uWidth", GL_FLOAT);
    this->uHeight_Location = this->getUniformLocation("uHeight", GL_FLOAT);
    this->uHeightBoost_Location = this->getUniformLocation("uHeightBoost", GL_INT);
    this->uColorTexture_Location = this->getUniformLocation("uColorTexture", GL_SAMPLER_2D);
    this->uHeightTexture_Location = this->getUniformLocation("uHeightTexture", GL_SAMPLER_2D);

    this->uLightPosition_Location = this->getUniformLocation("uLightPosition", GL_FLOAT_VEC3);
    this->uRenderShading_Location = this->getUniformLocation("uRenderShading", GL_BOOL);
    this->uRenderNormals_Location = this->getUniformLocation("uRenderNormals", GL_BOOL);
}

void ShaderTerrain::setMaterial(const Material& material) const {}

void ShaderTerrain::setDimensions(float width, float height) {
    this->setFloat(this->uWidth_Location, width);
    this->setFloat(this->uHeight_Location, height);
}

void ShaderTerrain::setHeightBoost(int height_boost) {
    this->setInt(this->uHeightBoost_Location, height_boost);
}

void ShaderTerrain::setLightPosition(const Math::Vec3& light_position) {
    this->setVec3(this->uLightPosition_Location, light_position);
}

void ShaderTerrain::setRenderShading(bool shading) {
    this->setBool(this->uRenderShading_Location, shading);
}

void ShaderTerrain::setRenderNormals(bool normals) {
    this->setBool(this->uRenderNormals_Location, normals);
}

void ShaderTerrain::setColorTexture(Texture color_texture) {
    this->use();
    glCheckError(glActiveTexture(GL_TEXTURE0 + 0));
    glCheckError(glBindTexture(GL_TEXTURE_2D, color_texture.id));
    glUniform1i(this->uColorTexture_Location, 0);
}

void ShaderTerrain::setHeightTexture(Texture height_texture) {
    this->use();
    glCheckError(glActiveTexture(GL_TEXTURE0 + 1));
    glCheckError(glBindTexture(GL_TEXTURE_2D, height_texture.id));
    glUniform1i(this->uHeightTexture_Location, 1);
}