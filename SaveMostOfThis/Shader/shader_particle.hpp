#pragma once

#include <vector>
#include <Shader/shader_base.hpp>

struct ShaderParticle : public ShaderBase {
    ShaderParticle() = default;
    ShaderParticle(DS::Vector<const char*> shader_paths);

    void compile() override;
    void setMaterial(const Material &material) const override;
    
    void setTexture(Texture texture) const;
private:
    unsigned int uTexture_Location;
};