#include <SFE/sfe.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

float dt = 0;
float WIDTH = 900;
float HEIGHT = 900;
float accumulator = 0;
Camera camera = Camera(0, 1, 10);
ShaderNoMaterial particle_shader;
GFX::Geometry particle;
DS::Vector<Math::Vec3> particle_centers;
DS::Vector<Math::Vec3> particle_colors;
GFX::GPUBuffer particle_center_buffer;
GFX::GPUBuffer particle_color_buffer;
bool mouse_captured = false;
Random::Seed seed;
Texture fire_texture;
Texture smoke_texture;
Texture galaxy_texture;
GLFWwindow* g_window;
bool toggle_gravity = true;
float time_scale = 1.0f;
float singularity_mass = 1000000.0f;

Math::Vec3 get_gravity_force(Math::Vec3 position_a, float mass_a, Math::Vec3 position_b, float mass_b) {
    if (!toggle_gravity) return Math::Vec3(0.0);

    Math::Vec3 AB = (position_b - position_a).normalize();
    float distance = Math::Vec3::Distance(position_a, position_b);
    float force_magnitude = (float)(1 * ((mass_a * mass_b) / SQUARED(distance + 50)));
    Math::Vec3 gravity_force = AB.scale(force_magnitude);

    return gravity_force;
}


struct ParticleRange {
    int start_index;
    int length;
};

#define THREAD_COUNT 5

struct ThreadSystem {
    std::mutex mtx;
    std::condition_variable cv_start;
    std::condition_variable cv_done;
    std::atomic<int> completed{0};
    std::thread threads[THREAD_COUNT];
    ParticleRange ranges[THREAD_COUNT];
};

ThreadSystem g_threads;

struct Particle {
    Math::Vec3 scale;
    Math::Vec3 position;
    Math::Vec3 velocity;
    Math::Vec3 acceleration;
    Math::Vec3 force;
    float mass = 0.1f;
};

const int MAX_PARTICLES = 100000;
DS::Vector<Particle> particles;
int particle_count = 0;


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
    // const bool SHIFT = Input::GetKey(Input::KEY_SHIFT, Input::PRESSED|Input::DOWN);

    if (Input::GetKeyPressed(Input::KEY_ESCAPE)) {
        glfwSetWindowShouldClose(g_window, true);
    }

    if (Input::GetKeyPressed(Input::KEY_R)) {
        particle_shader.compile();
    }

    if (Input::GetKeyPressed(Input::KEY_G)) {
        toggle_gravity = !toggle_gravity;
        LOG_TRACE("gravity: %s\n", singularity_mass ? "ON" : "OFF");
    }

    if (Input::GetKeyDown(Input::KEY_LEFT)) {
        singularity_mass -= 10000.0f;
        LOG_TRACE("mass: %f\n", singularity_mass);
    }
    
    if (Input::GetKeyDown(Input::KEY_RIGHT)) {
        singularity_mass += 10000.0f;
        LOG_TRACE("mass: %f\n", singularity_mass);
    }

    if (Input::GetKeyPressed(Input::KEY_UP)) {
        time_scale += 0.1f;
        LOG_TRACE("time: %f\n", time_scale);
    }
    
    if (Input::GetKeyPressed(Input::KEY_DOWN)) {
        time_scale -= 0.1f;
        LOG_TRACE("time: %f\n", time_scale);
    }

    if (Input::GetKeyPressed(Input::KEY_L)) {
        GFX::SetWireFrame(true);
    } else if (Input::GetKeyReleased(Input::KEY_L)) {
        GFX::SetWireFrame(false);
    }

    if (Input::GetKeyPressed(Input::KEY_C)) {
        mouse_captured = !mouse_captured;
        glfwSetInputMode(g_window, GLFW_CURSOR, mouse_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
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

// TODO(Jovanni):
// investigate why this little update function takes all the cpu time.
// There must be tons of unecessary call instructions.

// Also make a new branch called windows-multi-thread-update and put this update on a seperate thread.
// Then on each frame of the update set a condition (update is complete), make sure to check for spurious wakeups
void update_worker(int index) {
    ParticleRange pr = g_threads.ranges[index];
    while (!glfwWindowShouldClose(g_window)) {
        {
            std::unique_lock<std::mutex> lock(g_threads.mtx);
            g_threads.cv_start.wait(lock, [] { 
                return g_threads.completed == 0; 
            });
        }

        Math::Vec3 singularity_position = Math::Vec3(500, 500, 500);
        for (int i = pr.start_index; i < pr.start_index + pr.length; i++) {
            Particle* p = &particles[i];
            p->acceleration = p->force.scale(1.0f / p->mass);
            p->velocity += p->acceleration.scale(dt);
            p->position += p->velocity.scale(dt);
            p->velocity = p->velocity.scale(0.9998f);
            p->force = get_gravity_force(p->position, p->mass, singularity_position, singularity_mass);
            
            particle_centers[i] = p->position;
            particle_colors[i] = p->velocity.scale(1.0f / 6.0f);
        }

        if (++g_threads.completed >= THREAD_COUNT) {
            std::lock_guard<std::mutex> lock(g_threads.mtx);
            g_threads.cv_done.notify_one();
        }
    }
}

void render() {
    {
        std::lock_guard<std::mutex> lock(g_threads.mtx);
        g_threads.completed = 0;
    }
    g_threads.cv_start.notify_all();

    {
        std::unique_lock<std::mutex> lock(g_threads.mtx);
        g_threads.cv_done.wait(lock, [] { 
            return g_threads.completed >= THREAD_COUNT; 
        });
    }

    glClearColor(0.2f, 0.2f, 0.2f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Math::Mat4 perspective = GFX::GetProjectionMatrix3D(WIDTH, HEIGHT, camera.zoom);
    Math::Mat4 view = camera.getViewMatrix();

    particle_shader.setView(view);
    particle_shader.setProjection(perspective);

    particle.VAO.bind();
    particle_center_buffer.updateEntireBuffer(particle_centers);
    particle_color_buffer.updateEntireBuffer(particle_colors);
    particle_shader.setTexture2D("uTexture", 0, smoke_texture);
    particle.drawInstanced(&particle_shader, particle_count);
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
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT); 
    // glEnable(GL_FRAMEBUFFER_SRGB);

    return window;
}

int main(int argc, char** argv) {
    seed = Random::GenerateSeed(451);
    Memory::GeneralAllocator allocator = Memory::GeneralAllocator();
    Memory::bindAllocator(&allocator);

    g_window = GLFW_INIT();
    Input::Init();
    if (!Input::GLFW_SETUP(g_window)) {
        LOG_ERROR("Failed to setup GLFW\n");
        glfwTerminate();
        exit(-1);
    }
    Input::GLFW_BIND_MOUSE_MOVE_CALLBACK(mouse);

    #define MASTER_PROFILE "master"
    #define MOVEMENT_PROFILE "movement"
    Input::CreateProfile(MASTER_PROFILE, cbMasterProfile);
    Input::CreateProfile(MOVEMENT_PROFILE, cbMovementProfile);

    particle = GFX::Geometry::Quad();
    particle_shader = ShaderNoMaterial({"../../Scenes/ParticleScene/Shaders/Particle/particle.vert", "../../Scenes/ParticleScene/Shaders/Particle/particle.frag"});
    particles = DS::Vector<Particle>(MAX_PARTICLES, MAX_PARTICLES);
    particle_centers = DS::Vector<Math::Vec3>(MAX_PARTICLES, MAX_PARTICLES);
    particle_colors = DS::Vector<Math::Vec3>(MAX_PARTICLES, MAX_PARTICLES);

    particle_center_buffer = GFX::GPUBuffer::VBO(
        GFX::BufferUsage::DYNAMIC,
        {GFX::AttributeDesc(0, GFX::BufferStrideTypeInfo::VEC3)},
        particle_centers
    );

    particle_color_buffer = GFX::GPUBuffer::VBO(
        GFX::BufferUsage::DYNAMIC,
        {GFX::AttributeDesc(0, GFX::BufferStrideTypeInfo::VEC3)},
        particle_colors
    );

    particle.VAO.bindVBO(8, true, particle_center_buffer);
    particle.VAO.bindVBO(9, true, particle_color_buffer);

    fire_texture = Texture::LoadFromFile("../../Assets/Textures/fire.jpg"); 
    smoke_texture = Texture::LoadFromFile("../../Assets/Textures/smoke.jpg"); 
    galaxy_texture = Texture::LoadFromFile("../../Assets/Textures/galaxy.jpg");

    int start_index = 0;
    int length = MAX_PARTICLES / THREAD_COUNT;
    for (int i = 0; i < THREAD_COUNT; i++) {
        g_threads.ranges[i] = { start_index, length };
        start_index += length;
        g_threads.threads[i] = std::thread(update_worker, i);
    }

    float previous = glfwGetTime();

    while (!glfwWindowShouldClose(g_window)) {
        float current = glfwGetTime();
        dt = (current - previous) * time_scale;
        previous = current;
        accumulator += dt;
        
        Input::Poll();
        
        const float PARTICLE_SPAWN_COUNT_PER_SECOND = MAX_PARTICLES * 10;
        int spawn_count = (int)(PARTICLE_SPAWN_COUNT_PER_SECOND * dt);
        for (int i = 0; (particle_count < MAX_PARTICLES) && (i < spawn_count); i++) { 
            Particle p; 
            #if 1 
                float angle = 50.0f * accumulator + (i * 0.1f); 
                float speed = 2.0f + sin(accumulator); 
                float dx = speed * cos(angle); float dy = speed * sin(angle); 
                float dz = speed * sin(angle + i); 
            #elif 0 
                float freqX = 3.0f; 
                float freqY = 2.0f; 
                float strength = 4.0f; 
                float dx = strength * cos(freqX * accumulator + (i * 0.05f)); 
                float dy = strength * sin(freqY * accumulator); float dz = 0; 
            #else 
                float individualOffset = (float)next_available_particle_index * 0.001f; 
                float dx = 3.0f * cos(5.0f * accumulator + individualOffset); 
                float dy = 3.0f * sin(3.0f * accumulator + individualOffset); 
                float dz = 2.0f * sin(10.0f * accumulator);
            #endif 
            
            p.position = Math::Vec3(0, 0, 0); 
            p.scale = Math::Vec3(0.2f); 
            p.velocity = Math::Vec3(dx, dy, dz); 
            particles[particle_count++] = p; 
        }
        render();
        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }

    g_threads.completed = 0;
    g_threads.cv_start.notify_all();
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (g_threads.threads[i].joinable()) {
            g_threads.threads[i].join();
        }
    }

    glfwTerminate();
    return 0;
}
