#pragma once

#include <Shader/shader_base.hpp>

struct ShaderStencilOutline : public ShaderBase {
    ShaderStencilOutline() = default;
    ShaderStencilOutline(DS::Vector<const char*> shader_paths);

    void compile() override;
    void setMaterial(const Material &material) const override;
    
    void setOutlineScale(float s) const;
    
private:
    unsigned int uOutlineScale_Location = 0;
};