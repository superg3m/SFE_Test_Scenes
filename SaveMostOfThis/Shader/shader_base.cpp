#include "shader_base.hpp"

#include <DataStructure/ds.hpp>
#include <String/string.hpp>
#include <platform.hpp>
#include <renderer.hpp>

static const char* glEnumToString(GLenum type) {
    switch (type) {
        case GL_BOOL:          return "bool";
        case GL_INT:           return "int";
        case GL_FLOAT:         return "float";
        case GL_FLOAT_VEC2:    return "vec2";
        case GL_FLOAT_VEC3:    return "vec3";
        case GL_FLOAT_VEC4:    return "vec4";
        case GL_FLOAT_MAT4:    return "mat4";
        case GL_SAMPLER_2D:    return "sampler2D";
        case GL_SAMPLER_CUBE:  return "samplerCube";
        
        default:               return "unknown";
    }
}

GLenum ShaderBase::typeFromPath(const char* shader_source_path) {
    u64 shader_path_length = String::Length(shader_source_path);
    s64 extension_index = String::LastIndexOf(shader_source_path, shader_path_length, STRING_LIT_ARG("."));
    RUNTIME_ASSERT_MSG(extension_index != -1, "Missing extension (.vert, .frag)\n");
    DS::View<char> extension = DS::View<char>(shader_source_path + extension_index, shader_path_length - (u64)extension_index);

    if (String::Contains(extension.data, extension.length, STRING_LIT_ARG(".vert"))) {
        return GL_VERTEX_SHADER;
    } else if (String::Contains(extension.data, extension.length, STRING_LIT_ARG(".frag"))) {
        return GL_FRAGMENT_SHADER;
    } else if (String::Contains(extension.data, extension.length, STRING_LIT_ARG(".gs"))) {
        return GL_GEOMETRY_SHADER;
    }

    RUNTIME_ASSERT_MSG(false, "Unsupported extension: %.*s | Expected: (.vert, .frag, .gs)\n", extension.length, extension.data);
    return GL_INVALID_ENUM;
}

void ShaderBase::checkCompileError(unsigned int source_id, const char* path) {
    int success;
    char info_log[1024];
    glGetShaderiv(source_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(source_id, 1024, NULL, info_log);
        LOG_ERROR("ERROR::SHADER_COMPILATION_ERROR {%s}\n", path);
        LOG_ERROR("%s -- --------------------------------------------------- --\n", info_log);
    }
}

unsigned int ShaderBase::shaderSourceCompile(const char* path) {
    byte_t file_size = 0;
    Error error = Error::SUCCESS;
    GLchar* shader_source = (GLchar*)Platform::ReadEntireFile(path, file_size, error);
    RUNTIME_ASSERT_MSG(error == Error::SUCCESS, "Shader Error: %s\n", path, getErrorString(error));

    GLenum type = this->typeFromPath(path);
    unsigned int source_id = glCreateShader(type);
    glShaderSource(source_id, 1, &shader_source, NULL);
    glCompileShader(source_id);

    this->checkCompileError(source_id, path);

    Memory::Free(shader_source);
    return source_id;
}

unsigned int ShaderBase::getUniformLocation(const char* name, GLenum type) const {
    if (!this->uniforms.has(name)) {
       // LOG_WARN("Shader {%s} Uniform: '%s' is not cached\n", this->shader_paths[0], name);
    } else {
        GLenum expected = this->uniforms.get(name);
        if (expected != type) {
            LOG_ERROR("Shader {%s} Uniform: '%s' type missmatch\n", this->shader_paths[0], name);
            LOG_ERROR("Expected: %s | Got: %s\n", glEnumToString(expected), glEnumToString(type));
            return (unsigned int)-1;
        }
    }
    
    GLint location = glGetUniformLocation(this->program_id, name);
    if (location == -1) {
        LOG_ERROR("Shader {%s} Uniform: '%s' does not exists\n", this->shader_paths[0], name);
    }

    return location;
}

unsigned int ShaderBase::createShaderProgram(DS::Vector<const char*> shader_paths) {
    unsigned int program_id = glCreateProgram();
    this->shader_paths = shader_paths;

    DS::Vector<unsigned int> shader_sourceIDs = DS::Vector<unsigned int>(shader_paths.count()); 
    for (const char* path : shader_paths) {
        unsigned int shader_source_id = this->shaderSourceCompile(path);
        glAttachShader(program_id, shader_source_id);
        shader_sourceIDs.push(shader_source_id);
    }
    glLinkProgram(program_id);

    GLint success = false;
    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[1028] = {0};
        glGetProgramInfoLog(program_id, 512, NULL, info_log);
        LOG_ERROR("LINKING_FAILED {%s}\n", shader_paths[0]);
        LOG_ERROR("%s -- --------------------------------------------------- --\n", info_log);
    }

    for (int i = 0; i < shader_sourceIDs.count(); i++) {
        glDeleteShader(shader_sourceIDs[i]);
    }

    for (auto entries : this->uniforms) {
        if (!entries.filled || entries.dead) continue;

        Memory::Free((char*)entries.key);
    }

    GLint uniform_count = 0;
    glGetProgramiv(program_id, GL_ACTIVE_UNIFORMS, &uniform_count);
    this->uniforms = DS::Hashmap<const char*, GLenum>(MAX(1, uniform_count));
    for (int i = 0; i < uniform_count; i++) {
        GLint size;
        GLenum type;
        const GLsizei name_max_size = 256;
        GLchar name[name_max_size];
        GLsizei name_length;
        glGetActiveUniform(program_id, (GLuint)i, name_max_size, &name_length, &size, &type, name);

        this->uniforms.put(String::Allocate(name, name_length), type);
    }

    return program_id;
}

void ShaderBase::use() const {
    Renderer::BindProgram(this->program_id);
}

void ShaderBase::setModel(Math::Mat4 &model) const {
    this->use();
    this->setMat4("uModel", model);
}

void ShaderBase::setView(Math::Mat4 &view) const {
    this->use();
    this->setMat4("uView", view);
}

void ShaderBase::setProjection(Math::Mat4 &projection) const {
    this->use();
    this->setMat4("uProjection", projection);
}

void ShaderBase::setBool(const char* name, bool value) const {
    this->use();
    glCheckError(glUniform1i(this->getUniformLocation(name, GL_BOOL), (int)value));
}
void ShaderBase::setInt(const char* name, int value) const {
    this->use();
    glCheckError(glUniform1i(this->getUniformLocation(name, GL_INT), value));
}
void ShaderBase::setFloat(const char* name, float value) const {
    this->use();
    glCheckError(glUniform1f(this->getUniformLocation(name, GL_FLOAT), value));
}
void ShaderBase::setVec2(const char* name, const Math::Vec2& value) const {
    this->use();
    glCheckError(glUniform2fv(this->getUniformLocation(name, GL_FLOAT_VEC2), 1, &value.x));
}
void ShaderBase::setVec2(const char* name, float x, float y) const {
    this->use();
    glCheckError(glUniform2f(this->getUniformLocation(name, GL_FLOAT_VEC2), x, y));
}
void ShaderBase::setVec3(const char* name, const Math::Vec3& value) const {
    this->use();
    glCheckError(glUniform3fv(this->getUniformLocation(name, GL_FLOAT_VEC3), 1, &value.x));
}
void ShaderBase::setVec3(const char* name, float x, float y, float z) const {
    this->use();
    glCheckError(glUniform3f(this->getUniformLocation(name, GL_FLOAT_VEC3), x, y, z));
}
void ShaderBase::setVec4(const char* name, const Math::Vec4& value) const {
    this->use();
    glCheckError(glUniform4fv(this->getUniformLocation(name, GL_FLOAT_VEC4), 1, &value.x));
}
void ShaderBase::setVec4(const char* name, float x, float y, float z, float w) const {
    this->use();
    glCheckError(glUniform4f(this->getUniformLocation(name, GL_FLOAT_VEC4), x, y, z, w));
}
void ShaderBase::setMat4(const char* name, const Math::Mat4& mat) const {
    this->use();
    glCheckError(glUniformMatrix4fv(this->getUniformLocation(name, GL_FLOAT_MAT4), 1, GL_TRUE, &mat.v[0].x));
}

// protected
void ShaderBase::setBool(unsigned int location, bool value) const {
    this->use();
    glCheckError(glUniform1i(location, (int)value));
}
void ShaderBase::setInt(unsigned int location, int value) const {
    this->use();
    glCheckError(glUniform1i(location, value));
}
void ShaderBase::setFloat(unsigned int location, float value) const {
    this->use();
    glCheckError(glUniform1f(location, value));
}
void ShaderBase::setVec2(unsigned int location, const Math::Vec2& value) const {
    this->use();
    glCheckError(glUniform2fv(location, 1, &value.x));
}
void ShaderBase::setVec2(unsigned int location, float x, float y) const {
    this->use();
    glCheckError(glUniform2f(location, x, y));
}
void ShaderBase::setVec3(unsigned int location, const Math::Vec3& value) const {
    this->use();
    glCheckError(glUniform3fv(location, 1, &value.x));
}
void ShaderBase::setVec3(unsigned int location, float x, float y, float z) const {
    this->use();
    glCheckError(glUniform3f(location, x, y, z));
}
void ShaderBase::setVec4(unsigned int location, const Math::Vec4& value) const {
    this->use();
    glCheckError(glUniform4fv(location, 1, &value.x));
}
void ShaderBase::setVec4(unsigned int location, float x, float y, float z, float w) const {
    this->use();
    glCheckError(glUniform4f(location, x, y, z, w));
}
void ShaderBase::setMat4(unsigned int location, const Math::Mat4& mat) const {
    this->use();
    glCheckError(glUniformMatrix4fv(location, 1, GL_TRUE, &mat.v[0].x));
}