#pragma once

#include <SFE/Renderer/ShaderBase/sfe_shader_base.hpp>

struct ShaderQuadInstance : public ShaderBase {
    ShaderQuadInstance() = default;
    ShaderQuadInstance(DS::Vector<const char*> shader_paths);
    void compile() override;
    void setMaterial(const Material &material) const override;
};