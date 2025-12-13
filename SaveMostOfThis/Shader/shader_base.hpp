#pragma once

#include <vector>
#include <string>

#include <glad/glad.h>

#include <DataStructure/ds.hpp>
#include <Geometry/material.hpp>
#include <Math/matrix.hpp>

struct ShaderBase {
    unsigned int program_id = 0;

    virtual void compile() = 0;
    virtual void setMaterial(const Material &material) const = 0;

    void use() const;
    void setModel(Math::Mat4 &model) const;
    void setView(Math::Mat4 &view) const;
    void setProjection(Math::Mat4 &projection) const;

    void setBool(const char* name, bool value) const;
    void setInt(const char* name, int value) const;
    void setFloat(const char* name, float value) const;
    void setVec2(const char* name, const Math::Vec2& value) const;
    void setVec2(const char* name, float x, float y) const;
    void setVec3(const char* name, const Math::Vec3& value) const;
    void setVec3(const char* name, float x, float y, float z) const;
    void setVec4(const char* name, const Math::Vec4& value) const;
    void setVec4(const char* name, float x, float y, float z, float w) const;
    void setMat4(const char* name, const Math::Mat4& mat) const;
protected:
    DS::Vector<const char*> shader_paths;
    DS::Hashmap<const char*, GLenum> uniforms;

    ShaderBase() = default;
    GLenum typeFromPath(const char* path);
    void checkCompileError(unsigned int source_id, const char* path);
    unsigned int shaderSourceCompile(const char* path);
    unsigned int getUniformLocation(const char* name, GLenum type) const;
    unsigned int createShaderProgram(DS::Vector<const char*> shader_paths);

    void setBool(unsigned int location, bool value) const;
    void setInt(unsigned int location, int value) const;
    void setFloat(unsigned int location, float value) const;
    void setVec2(unsigned int location, const Math::Vec2& value) const;
    void setVec2(unsigned int location, float x, float y) const;
    void setVec3(unsigned int location, const Math::Vec3& value) const;
    void setVec3(unsigned int location, float x, float y, float z) const;
    void setVec4(unsigned int location, const Math::Vec4& value) const;
    void setVec4(unsigned int location, float x, float y, float z, float w) const;
    void setMat4(unsigned int location, const Math::Mat4& mat) const;
};