#pragma once

#include <SFE/GFX/ShaderBase/sfe_shader_base.hpp>

struct ShaderPlanet : public ShaderBase {
    ShaderPlanet() = default;
    ShaderPlanet(DS::Vector<const char*> shader_paths);
    void compile() override;
    void setMaterial(const Material &material) const override;
    void setTexture(Texture &texture);

private:
    unsigned int uTexture_Location;
};