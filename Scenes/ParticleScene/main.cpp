#include <SFE/sfe.hpp>

float dt = 0;
float WIDTH = 900;
float HEIGHT = 900;
float accumulator = 0;
Camera camera = Camera(0, 1, 10);
ShaderNoMaterial particle_shader;
GFX::Geometry particle;
DS::Vector<Math::Mat4> particle_models;
GFX::GPUBuffer particle_model_buffer;
bool mouse_captured = false;
Random::Seed seed;
Texture fire_texture;

#define MASTER_PROFILE "master"
#define MOVEMENT_PROFILE "movement"

struct Particle {
    Math::Vec3 scale = Math::Vec3(0.1f);
    Math::Vec3 position =  Math::Vec3(0.0f);
    Math::Quat orientation = Math::Quat::Identity();

    float angular_velocity_z = 720; // 720 degree rotation per second
    Math::Vec3 velocity = Math::Vec3(0, 1, 0);
    // float lifetime = 0.0f;

    // float color;
    // Math::Quat orientation;
    // Math::Vec3 scale;
    // Geometry geometry;
    // Math::Vec3 acceleration;
};

const int MAX_PARTICLES = 10000;
Particle particles[MAX_PARTICLES];
u32 next_available_particle_index;

void mouse(GLFWwindow* window, double mouse_x, double mouse_y) {
    static bool first = true;
    static float last_mouse_x;
    static float last_mouse_y;

    if (first) {
        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;
        first = false;
        return;
    }

    float xoffset = mouse_x - last_mouse_x;
    float yoffset = last_mouse_y - mouse_y;

    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;

    if (mouse_captured) {
        camera.processMouseMovement(xoffset, yoffset);
    }
}


void cbMasterProfile() {
    GLFWwindow* window = (GLFWwindow*)Input::glfw_window_instance;
    // const bool SHIFT = Input::GetKey(Input::KEY_SHIFT, Input::PRESSED|Input::DOWN);

    if (Input::GetKeyPressed(Input::KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }

    if (Input::GetKeyPressed(Input::KEY_R)) {
        particle_shader.compile();
    }

    if (Input::GetKeyPressed(Input::KEY_L)) {
        GFX::SetWireFrame(true);
    } else if (Input::GetKeyReleased(Input::KEY_L)) {
        GFX::SetWireFrame(false);
    }

    if (Input::GetKeyPressed(Input::KEY_C)) {
        mouse_captured = !mouse_captured;
        glfwSetInputMode(window, GLFW_CURSOR, mouse_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

void cbMovementProfile() {
    if (Input::GetKey(Input::KEY_SPACE, Input::PRESSED|Input::DOWN)) {
        camera.processKeyboard(UP, dt);
    }

    if (Input::GetKey(Input::KEY_CTRL, Input::PRESSED|Input::DOWN)) {
        camera.processKeyboard(DOWN, dt);
    }

    if (Input::GetKey(Input::KEY_W, Input::PRESSED|Input::DOWN)) {
        camera.processKeyboard(FORWARD, dt); 
    }

    if (Input::GetKey(Input::KEY_A, Input::PRESSED|Input::DOWN)) {
        camera.processKeyboard(LEFT, dt); 
    }

    if (Input::GetKey(Input::KEY_S, Input::PRESSED|Input::DOWN)) {
        camera.processKeyboard(BACKWARD, dt); 
    }

    if (Input::GetKey(Input::KEY_D, Input::PRESSED|Input::DOWN)) {
        camera.processKeyboard(RIGHT, dt); 
    }
}

void update() {
    const float PARTICLE_SPAWN_COUNT_PER_FRAME = 20;
    for (int i = 0; i < PARTICLE_SPAWN_COUNT_PER_FRAME; i++) {
        Particle p;
        p.position = Math::Vec3(0, 0, 0);
        float dx = Random::GenerateRange(&seed, -1, 1);
        float dy = Random::GenerateRange(&seed, 1, 4);
        float dz = 0;
        p.velocity = Math::Vec3(dx, dy, dz);

        particles[next_available_particle_index] = p;
        next_available_particle_index = (next_available_particle_index + 1) % MAX_PARTICLES;
    }

    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle* p = &particles[i];
        p->position += p->velocity.scale(dt);
        p->orientation = Math::Quat::FromEuler(0, 0, p->angular_velocity_z * accumulator);
        particle_models[i] = Math::Mat4::Transform(p->scale, p->orientation, p->position);;
    }
}

void render() {
    glClearColor(0.2f, 0.2f, 0.2f, 0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    Math::Mat4 perspective = GFX::GetProjectionMatrix3D(WIDTH, HEIGHT, camera.zoom);
    Math::Mat4 view = camera.getViewMatrix();

    particle_shader.setView(view);
    particle_shader.setProjection(perspective);

    particle.VAO.bind();
    particle_model_buffer.updateEntireBuffer(particle_models);
    particle_shader.setTexture2D("uTexture", 0, fire_texture);
    particle.drawInstanced(&particle_shader, MAX_PARTICLES);
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
    
    Input::GLFW_BIND_MOUSE_MOVE_CALLBACK(mouse);

    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, mouse_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

    glEnable(GL_MULTISAMPLE);
    GFX::SetDepthTest(true);
    GFX::SetStencilTest(true);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_FRAMEBUFFER_SRGB);

    return window;
}

int main(int argc, char** argv) {
    seed = Random::GenerateSeed(451);
    Memory::GeneralAllocator allocator = Memory::GeneralAllocator();
    Memory::bindAllocator(&allocator);

    GLFWwindow* window = GLFW_INIT();
    Input::Init();
    if (!Input::GLFW_SETUP(window)) {
        LOG_ERROR("Failed to setup GLFW\n");
        glfwTerminate();
        exit(-1);
    }

    Input::CreateProfile(MASTER_PROFILE, cbMasterProfile);
    Input::CreateProfile(MOVEMENT_PROFILE, cbMovementProfile);
    
    particle = GFX::Geometry::Quad();
    particle_shader = ShaderNoMaterial({"../../Scenes/ParticleScene/Shaders/Particle/particle.vert", "../../Scenes/ParticleScene/Shaders/Particle/particle.frag"});
    particle_models = DS::Vector<Math::Mat4>(MAX_PARTICLES, MAX_PARTICLES);

    particle_model_buffer = GFX::GPUBuffer::VBO(GFX::BufferUsage::DYNAMIC, {GFX::AttributeDesc(0, GFX::BufferStrideTypeInfo::MAT4)}, particle_models);
    particle.VAO.bindVBO(8, true, particle_model_buffer);

    fire_texture = Texture::LoadFromFile("../../Assets/Textures/fire.jpg");

    float previous = 0;
    float timer = 2;
	while (!glfwWindowShouldClose(window)) {
        float current = glfwGetTime();
        dt = current - previous;
        previous = current;

        accumulator += dt;

        if (timer == 0) {
            timer = 2;
            LOG_DEBUG("FPS: %d\n", (int)(1.0f / dt));
        } else {
            timer = Math::MoveToward(timer, 0, dt);
        }

        Input::Poll();

        update();
		render();

		glfwPollEvents();
        glfwSwapBuffers(window);
	}

	exit(EXIT_SUCCESS);
}