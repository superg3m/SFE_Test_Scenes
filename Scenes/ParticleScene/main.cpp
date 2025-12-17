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
    float force_magnitude = (float)((mass_a * mass_b) / SQUARED(distance + 50));
    return AB.scale(force_magnitude);
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
    bool start_update = false;
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

void update_worker(int index) {
    ParticleRange pr = g_threads.ranges[index];
    while (!glfwWindowShouldClose(g_window)) {
        {
            std::unique_lock<std::mutex> lock(g_threads.mtx);
            g_threads.cv_start.wait(lock, [] { 
                return g_threads.start_update; 
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
        g_threads.start_update = true;
    }
    g_threads.cv_start.notify_all();

    {
        std::unique_lock<std::mutex> lock(g_threads.mtx);
        g_threads.cv_done.wait(lock, [] { 
            return g_threads.completed >= THREAD_COUNT; 
        });
        g_threads.start_update = false;
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

int main(int argc, char** argv) {
    seed = Random::GenerateSeed(451);
    Memory::GeneralAllocator allocator = Memory::GeneralAllocator();
    Memory::bindAllocator(&allocator);

    glfwInit();
    g_window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
    glfwMakeContextCurrent(g_window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

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

        const float PARTICLE_SPAWN_COUNT_PER_SECOND = 3000;
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

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (g_threads.threads[i].joinable())
            g_threads.threads[i].join();
    }

    glfwTerminate();
    return 0;
}
