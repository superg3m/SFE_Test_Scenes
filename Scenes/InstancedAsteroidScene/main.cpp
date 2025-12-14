#include <SFE/sfe.hpp>

#include "Shaders/QuadInstance/quad.hpp"

Camera camera = Camera(0, 1, 10);
float WIDTH = 900;
float HEIGHT = 900;

GFX::Geometry quad;
ShaderQuadInstance instance_shader;
DS::Vector<Math::Vec2> translations;

void render() {
    glClearColor(0.2f, 0.2f, 0.2f, 0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    instance_shader.use();
    quad.drawInstanced(&instance_shader, 100);

    GFX::ClearTelemetry();
}

void init_geometry() {
    DS::Vector<GFX::Vertex> quad_vertices = {
        Math::Vec3(-0.05f, +0.05f, 0),
        Math::Vec3(+0.05f, -0.05f, 0),
        Math::Vec3(-0.05f, -0.05f, 0),

        Math::Vec3(-0.05f, +0.05f, 0),
        Math::Vec3(+0.05f, -0.05f, 0),
        Math::Vec3(+0.05f, +0.05f, 0),
    };

    quad = GFX::Geometry(quad_vertices);

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

    DS::Vector<GFX::AttributeDesc> descriptors = {GFX::AttributeDesc(8, true, 0, GFX::BufferStrideTypeInfo::VEC2)};
    GFX::GPUBuffer instanced_offset_buffer = GFX::GPUBuffer::VBO(GFX::BufferUsage::STATIC, descriptors, translations);
    quad.VAO.bindBuffer(instanced_offset_buffer);
}

GLFWwindow* GLFW_INIT() {
    RUNTIME_ASSERT_MSG(glfwInit(), "Failed to init glfw\n");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        LOG_ERROR("Failed to create GLFW window\n");
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD\n");
        glfwTerminate();
        exit(-1);
    }


    glfwSwapInterval(1); // vsync
    glEnable(GL_MULTISAMPLE);
    GFX::SetDepthTest(true);
    GFX::SetStencilTest(true);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_FRAMEBUFFER_SRGB);

    return window;
}

int main(int argc, char** argv) {
    Random::Seed seed = Random::GenerateSeed(451);
    Memory::GeneralAllocator allocator = Memory::GeneralAllocator();
    Memory::bindAllocator(&allocator);

    GLFWwindow* window = GLFW_INIT();
    Input::Init();
    if (!Input::GLFW_SETUP(window)) {
        LOG_ERROR("Failed to setup GLFW\n");
        glfwTerminate();
        exit(-1);
    }
    
    instance_shader = ShaderQuadInstance({"../../Scenes/InstancedAsteroidScene/Shaders/QuadInstance/quad.vert", "../../Scenes/InstancedAsteroidScene/Shaders/QuadInstance/quad.frag"});

    init_geometry();

    while (!glfwWindowShouldClose(window)) {
        Input::Poll();

        render();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    exit(EXIT_SUCCESS);
}

/*
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0f); 
}
*/