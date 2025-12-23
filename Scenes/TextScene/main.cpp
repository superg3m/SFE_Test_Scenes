#include <SFE/sfe.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

struct AppState {
    float dt = 0;
    float WIDTH = 900;
    float HEIGHT = 900;
    float accumulator = 0;
    ShaderNoMaterial text_shader; // probably need to make this Shader3DNoMaterial and Shader2DNoMaterial or something like this
    Random::Seed seed = Random::GenerateSeed(451);
    Memory::GeneralAllocator allocator = Memory::GeneralAllocator();
    GLFWwindow* window;

    AppState() {
        Memory::bindAllocator(&this->allocator);
    }
};

AppState $app; // Global I saw this once and I kind of like it...

void render() {
    glClearColor(0.2f, 0.2f, 0.2f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Math::Mat4 orthographic = GFX::GetProjectionMatrix2D($app.WIDTH, $app.HEIGHT);
    $app.text_shader.setProjection(orthographic);
    // particle.drawInstanced(&particle_shader, particle_count);
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

    GLFWwindow* window = glfwCreateWindow($app.WIDTH, $app.HEIGHT, "LearnOpenGL", nullptr, nullptr);
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

    glfwSwapInterval(1);

    GFX::SetDepthTest(true);
    GFX::SetStencilTest(true);

    return window;
}

int main(int argc, char** argv) {
    $app.window = GLFW_INIT();
    Input::Init();
    if (!Input::GLFW_SETUP($app.window)) {
        LOG_ERROR("Failed to setup GLFW\n");
        glfwTerminate();
        exit(-1);
    }

    // particle = GFX::Geometry::Quad();
    // particle_shader = ShaderNoMaterial({"../../Scenes/ParticleScene/Shaders/Particle/particle.vert", "../../Scenes/ParticleScene/Shaders/Particle/particle.frag"});

    float previous = 0;
    float timer = 2;
	while (!glfwWindowShouldClose($app.window)) {
        float current = glfwGetTime();
        float dt_for_fps = (current - previous);
        $app.dt = (current - previous);
        previous = current;

        $app.accumulator += $app.dt;
        if (timer == 0) {
            timer = 2;
            LOG_DEBUG("FPS: %d\n", (int)(1.0f / dt_for_fps));
        } else {
            timer = Math::MoveToward(timer, 0, dt_for_fps);
        }
        
        Input::Poll();
        render();
        
        glfwSwapBuffers($app.window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
