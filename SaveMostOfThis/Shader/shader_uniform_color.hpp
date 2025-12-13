#pragma once

#include <Shader/shader_base.hpp>

struct ShaderUniformColor : public ShaderBase {
    ShaderUniformColor() = default;
    ShaderUniformColor(DS::Vector<const char*> shader_paths);
    void compile() override;
    void setMaterial(const Material &material) const override;

    void setColor(Math::Vec3 color);

private:
    unsigned int uColor_Location = 0;
};