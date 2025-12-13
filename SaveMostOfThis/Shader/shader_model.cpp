#include <String/string.hpp>
#include <Shader/shader_model.hpp>
#include <glad/glad.h>
#include <renderer.hpp>

ShaderModel::ShaderModel(DS::Vector<const char*> shader_paths) {
    this->shader_paths = shader_paths;
    this->compile();
}

void ShaderModel::compile() {
    this->program_id = this->createShaderProgram(this->shader_paths);

    this->uApplyEmissiveMaterial_Location = this->getUniformLocation("uApplyEmissiveMaterial", GL_BOOL);
    this->uCameraPosition_Location = this->getUniformLocation("uCameraPosition", GL_FLOAT_VEC3);

    this->uMaterial_Location.textures[TEXTURE_TYPE_DIFFUSE] = this->getUniformLocation("uMaterial.diffuse_map", GL_SAMPLER_2D);
    this->uMaterial_Location.has_textures[TEXTURE_TYPE_DIFFUSE] = this->getUniformLocation("uMaterial.has_diffuse_map", GL_BOOL);

    this->uMaterial_Location.textures[TEXTURE_TYPE_SPECULAR] = this->getUniformLocation("uMaterial.specular_map", GL_SAMPLER_2D);
    this->uMaterial_Location.has_textures[TEXTURE_TYPE_SPECULAR] = this->getUniformLocation("uMaterial.has_specular_map", GL_BOOL);

    this->uMaterial_Location.textures[TEXTURE_TYPE_EMISSIVE] = this->getUniformLocation("uMaterial.emissive_map", GL_SAMPLER_2D);
    this->uMaterial_Location.has_textures[TEXTURE_TYPE_EMISSIVE] = this->getUniformLocation("uMaterial.has_emissive_map", GL_BOOL);

    this->uMaterial_Location.ambient_color = this->getUniformLocation("uMaterial.ambient_color", GL_FLOAT_VEC3);
    this->uMaterial_Location.diffuse_color = this->getUniformLocation("uMaterial.diffuse_color", GL_FLOAT_VEC3);
    this->uMaterial_Location.specular_color = this->getUniformLocation("uMaterial.specular_color", GL_FLOAT_VEC3);

    this->uMaterial_Location.shininess = this->getUniformLocation("uMaterial.shininess", GL_FLOAT);
    this->uMaterial_Location.opacity = this->getUniformLocation("uMaterial.opacity", GL_FLOAT);

    this->uMaterial_Location.has_normals = this->getUniformLocation("uMaterial.has_normals", GL_BOOL);
    this->uMaterial_Location.has_texcoord = this->getUniformLocation("uMaterial.has_texcoord", GL_BOOL);
}

void ShaderModel::setMaterial(const Material &material) const {
    this->use();

    for (int i = 0; i < TEXTURE_COUNT - 1; i++) {
        if (material.textures[i + 1].id <= 0) {
            continue;
        }
        
        if (this->uMaterial_Location.textures[i + 1] <= 0) {
            LOG_WARN("Not handling texture type: %s\n", texture_to_string[i + 1]);
            continue;
        }

        glCheckError(glActiveTexture(GL_TEXTURE0 + i));
        glCheckError(glBindTexture(GL_TEXTURE_2D, material.textures[i + 1].id));
        this->setInt(this->uMaterial_Location.textures[i + 1], i);
        this->setBool(this->uMaterial_Location.has_textures[i + 1], true);
    }

    this->setVec3(uMaterial_Location.ambient_color, material.ambient_color);
    this->setVec3(uMaterial_Location.diffuse_color, material.diffuse_color);
    this->setVec3(uMaterial_Location.specular_color, material.specular_color);

    this->setFloat(uMaterial_Location.shininess, material.shininess);
    this->setFloat(uMaterial_Location.opacity, material.opacity);

    this->setBool(this->uMaterial_Location.has_normals, material.has_normals);
    this->setBool(this->uMaterial_Location.has_texcoord, material.has_texcoord);
}

void ShaderModel::setEmissiveMaterial(bool should_emit) const {
    this->use();
    this->setBool(this->uApplyEmissiveMaterial_Location, should_emit);
}

void ShaderModel::setCameraPosition(Math::Vec3 &camera_position) const {
    this->use();
    this->setVec3(this->uCameraPosition_Location, camera_position);
}