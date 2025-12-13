#pragma once

#include <Shader/shader_base.hpp>

struct ShaderTerrain : public ShaderBase {
    ShaderTerrain() = default;
    ShaderTerrain(DS::Vector<const char*> shader_paths);
    void compile() override;
    void setMaterial(const Material &material) const override;

    void setDimensions(float width, float height);
    void setHeightBoost(int height_boost);
    void setColorTexture(Texture color_texture);
    void setHeightTexture(Texture height_texture);

    void setLightPosition(const Math::Vec3& light_position);
    void setRenderShading(bool shading);
    void setRenderNormals(bool normals);
private:
    unsigned int uWidth_Location = 0;
    unsigned int uHeight_Location = 0;
    unsigned int uHeightBoost_Location = 0;
    unsigned int uColorTexture_Location = 0;
    unsigned int uHeightTexture_Location = 0;

    unsigned int uLightPosition_Location = 0;
    unsigned int uRenderShading_Location = 0;
    unsigned int uRenderNormals_Location = 0;
};
