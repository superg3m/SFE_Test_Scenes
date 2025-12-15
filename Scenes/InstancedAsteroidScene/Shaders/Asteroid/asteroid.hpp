#pragma once

#include <SFE/GFX/ShaderBase/sfe_shader_base.hpp>

struct ShaderAsteroid : public ShaderBase {
    ShaderAsteroid() = default;
    ShaderAsteroid(DS::Vector<const char*> shader_paths);
    void compile() override;
    void setMaterial(const Material &material) const override;

    void setTexture(Texture &texture);
private:
    unsigned int uTexture_Location;
};