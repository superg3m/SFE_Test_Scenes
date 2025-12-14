#include <SFE/sfe.hpp>

#include "Shaders/QuadInstance/quad.hpp"

ShaderQuadInstance instance_shader;

Renderer::Geometry quad;
DS::Vector<Math::Vec2> translations;

void render() {
    instance_shader.use();
    quad.drawInstanced(&instance_shader, 100);
}

int main() {
    DS::Vector<Renderer::Vertex> quad_vertices = {
        Math::Vec3(-0.05f, +0.05f, 0),
        Math::Vec3(+0.05f, -0.05f, 0),
        Math::Vec3(-0.05f, -0.05f, 0),

        Math::Vec3(-0.05f, +0.05f, 0),
        Math::Vec3(+0.05f, -0.05f, 0),
        Math::Vec3(+0.05f, +0.05f, 0),
    };

    quad = Renderer::Geometry(quad_vertices);

    int index = 0;
    float offset = 0.1f;
    translations = DS::Vector<Math::Vec2>(100);
    translations.resize(100);
    for (int y = -10; y < 10; y += 2) {
        for (int x = -10; x < 10; x += 2) {
            Math::Vec2 translation;
            translation.x = (float)x / 10.0f + offset;
            translation.y = (float)y / 10.0f + offset;
            translations[index++] = translation;
        }
    }

    Renderer::GPUBuffer instanced_offset_buffer = Renderer::GPUBuffer::VBO(
        Renderer::BufferType::VERTEX, Renderer::BufferUsage::STATIC, sizeof(Math::Vec2), 
        {{0, false, 0, Renderer::BufferStrideTypeInfo::VEC2}}, 
        sizeof(Math::Vec2) * translations.count(), translations.data()
    );

    quad.VAO.bindBuffer(instanced_offset_buffer);
}