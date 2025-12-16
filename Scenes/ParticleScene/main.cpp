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
GLFWwindow* g_window;


// Windows specific code
struct ParticleRange {
    int start_index;
    int length;
};

#undef APIENTRY
#include <windows.h>
#define THREAD_COUNT 10
struct Win32Specific {
    SRWLOCK lock;
    HANDLE threads[THREAD_COUNT];
    ParticleRange ranges[THREAD_COUNT];
    CONDITION_VARIABLE cv_all_update_theads_done;
    CONDITION_VARIABLE cv_start_update_threads;
    volatile LONG update_thread_completion_count = 0;
    volatile bool start_update_threads = false;
};

Win32Specific g_win32;
// Use a semaphore for update() if you have multiple threads doing the update on a range
// https://learn.microsoft.com/en-us/windows/win32/sync/using-semaphore-objects

#define MASTER_PROFILE "master"
#define MOVEMENT_PROFILE "movement"

struct Particle {
    Math::Vec3 scale = Math::Vec3(0.0f);
    Math::Vec3 position =  Math::Vec3(0.0f);
    Math::Quat orientation = Math::Quat::Identity();

    float angular_velocity_z = 720; // 720 degree rotation per second
    Math::Vec3 velocity = Math::Vec3(0, 1, 0);
    // float lifetime = 0.0f;

    // float color;
    // Math::Quat orientations;
    // Math::Vec3 scale;
    // Geometry geometry;
    // Math::Vec3 acceleration;
};

const int MAX_PARTICLES = 100000;
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
    // const bool SHIFT = Input::GetKey(Input::KEY_SHIFT, Input::PRESSED|Input::DOWN);

    if (Input::GetKeyPressed(Input::KEY_ESCAPE)) {
        glfwSetWindowShouldClose(g_window, true);
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
DWORD WINAPI update(void* param) {
    ParticleRange* pr = (ParticleRange*)param;

    while (!glfwWindowShouldClose(g_window)) {
        // block here until render consumes
        AcquireSRWLockShared(&g_win32.lock);
        WakeConditionVariable(&g_win32.cv_start_update_threads);
        while (!g_win32.start_update_threads) {
            SleepConditionVariableSRW(&g_win32.cv_start_update_threads, &g_win32.lock, INFINITE, CONDITION_VARIABLE_LOCKMODE_SHARED);
        }
        ReleaseSRWLockShared(&g_win32.lock);

        // NOTE(Jovanni): This doesn't need a lock because its partitioned correctly
        for (int i = pr->start_index; i < pr->start_index + pr->length; i++) {
            Particle* p = &particles[i];
            p->scale = Math::Vec3(0.25);
            p->position += p->velocity.scale(dt);
            p->orientation = Math::Quat::FromEuler(0, 0, p->angular_velocity_z * accumulator);
            particle_models[i] = Math::Mat4::Transform(p->scale, p->orientation, p->position);;
        }

        if (InterlockedIncrement(&g_win32.update_thread_completion_count) == THREAD_COUNT) {
            WakeConditionVariable(&g_win32.cv_all_update_theads_done);
        }
    }

    return 0;
}

void render() {
    AcquireSRWLockExclusive(&g_win32.lock);
    g_win32.update_thread_completion_count = 0;
    g_win32.start_update_threads = true;
    ReleaseSRWLockExclusive(&g_win32.lock);
    WakeAllConditionVariable(&g_win32.cv_start_update_threads);

    AcquireSRWLockExclusive(&g_win32.lock);
    while (g_win32.update_thread_completion_count < THREAD_COUNT) {
        SleepConditionVariableSRW(&g_win32.cv_all_update_theads_done, &g_win32.lock, INFINITE, 0);
    }
    g_win32.start_update_threads = false;
    ReleaseSRWLockExclusive(&g_win32.lock);

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

    g_window = GLFW_INIT();
    Input::Init();
    if (!Input::GLFW_SETUP(g_window)) {
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


    InitializeSRWLock(&g_win32.lock);
    InitializeConditionVariable(&g_win32.cv_all_update_theads_done);
    InitializeConditionVariable(&g_win32.cv_start_update_threads);

    int start_index = 0; 
    const int LENGTH = MAX_PARTICLES / THREAD_COUNT;
    for(int i = 0; i < THREAD_COUNT; i++) {
        g_win32.ranges[i].start_index = start_index;
        g_win32.ranges[i].length = LENGTH;
        start_index += LENGTH;

        g_win32.threads[i] = CreateThread( 
            nullptr,
            0,
            update, 
            &g_win32.ranges[i],
            0,
            nullptr
        );
    }

    float previous = 0;
    float timer = 2;
	while (!glfwWindowShouldClose(g_window)) {
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

        const float PARTICLE_SPAWN_COUNT_PER_FRAME = 50;
        for (int i = 0; i < PARTICLE_SPAWN_COUNT_PER_FRAME; i++) {
            Particle p;
            p.position = Math::Vec3(0, 0, 0);
            float angle = 50.0f * accumulator + (i * 0.1f);
            float speed = 2.0f + sin(accumulator);

            float dx = speed * cos(angle);
            float dy = speed * sin(angle);
            p.velocity = Math::Vec3(dx, dy, 0);

            particles[next_available_particle_index] = p;
            next_available_particle_index = (next_available_particle_index + 1) % MAX_PARTICLES;
        }
		render();

		glfwPollEvents();
        glfwSwapBuffers(g_window);
	}

    AcquireSRWLockExclusive(&g_win32.lock);
    g_win32.update_thread_completion_count = 0;
    g_win32.start_update_threads = true;
    ReleaseSRWLockExclusive(&g_win32.lock);
    WakeAllConditionVariable(&g_win32.cv_start_update_threads);

    WaitForMultipleObjects(THREAD_COUNT, g_win32.threads, TRUE, INFINITE);
    for(int i = 0; i < THREAD_COUNT; i++) {
        CloseHandle(g_win32.threads[i]);
    }

	return 0;
}