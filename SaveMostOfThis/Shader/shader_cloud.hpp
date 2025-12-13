#pragma once

#include <SFE/Renderer/ShaderBase/sfe_shader_base.hpp>

struct ShaderCloud : public ShaderBase {
    ShaderCloud() = default;
    ShaderCloud(DS::Vector<const char*> shader_paths);
    void compile() override;
    void setMaterial(const Material &material) const override;

    void setCloudTexture(const Texture& cloud_texture);
private:
    unsigned int uCloudTexture_Location = 0;
};