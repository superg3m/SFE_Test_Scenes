#include <SFE/sfe.hpp>

#define MASTER_PROFILE "master"
#define MOVEMENT_PROFILE "movement"

GFX::Geometry church;
ShaderMaterial church_shader;

GFX::Geometry skybox_geo;
ShaderNoMaterial skybox_shader;
Texture sky_cubemap;

// PointLight point_light;

struct ApplicationState {
    float dt = 0;
    float accumulator = 0;
    float WIDTH = 900;
    float HEIGHT = 900;

    bool emit = false;
    bool mouse_captured = true;
    bool render_shading = true;
    bool render_normals = false;

    Camera camera = Camera(0, 1, 10);
};

ApplicationState app;

void cbMasterProfile() {
    GLFWwindow* window = (GLFWwindow*)Input::glfw_window_instance;
    const bool SHIFT = Input::GetKey(Input::KEY_SHIFT, Input::PRESSED|Input::DOWN);

    if (Input::GetKeyPressed(Input::KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }

    if (Input::GetKeyPressed(Input::KEY_R)) {
        skybox_shader.compile();
    }

    if (Input::GetKeyPressed(Input::KEY_0)) {
        app.emit = !app.emit;
        app.render_shading = !app.render_shading;
    }

    if (Input::GetKeyPressed(Input::KEY_N)) {
        app.render_normals = !app.render_normals;
    }

    if (Input::GetKeyPressed(Input::KEY_L)) {
        GFX::SetWireFrame(true);
    } else if (Input::GetKeyReleased(Input::KEY_L)) {
        GFX::SetWireFrame(false);
    }

    if (Input::GetKeyPressed(Input::KEY_C)) {
        app.mouse_captured = !app.mouse_captured;
        glfwSetInputMode(window, GLFW_CURSOR, app.mouse_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
}

void cbMovementProfile() {
    if (Input::GetKey(Input::KEY_SPACE, Input::PRESSED|Input::DOWN)) {
        app.camera.processKeyboard(UP, app.dt);
    }

    if (Input::GetKey(Input::KEY_CTRL, Input::PRESSED|Input::DOWN)) {
        app.camera.processKeyboard(DOWN, app.dt);
    }

    if (Input::GetKey(Input::KEY_W, Input::PRESSED|Input::DOWN)) {
        app.camera.processKeyboard(FORWARD, app.dt); 
    }

    if (Input::GetKey(Input::KEY_A, Input::PRESSED|Input::DOWN)) {
        app.camera.processKeyboard(LEFT, app.dt); 
    }

    if (Input::GetKey(Input::KEY_S, Input::PRESSED|Input::DOWN)) {
        app.camera.processKeyboard(BACKWARD, app.dt); 
    }

    if (Input::GetKey(Input::KEY_D, Input::PRESSED|Input::DOWN)) {
        app.camera.processKeyboard(RIGHT, app.dt); 
    }
}

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

    if (app.mouse_captured) {
        app.camera.processMouseMovement(xoffset, yoffset);
    }
}

void update() {

}

void display() {
    glClearColor(0.2f, 0.2f, 0.2f, 0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    Math::Mat4 model = Math::Mat4::Identity();
    Math::Mat4 perspective = GFX::GetProjectionMatrix3D(app.WIDTH, app.HEIGHT, app.camera.zoom);
    Math::Mat4 view = app.camera.getViewMatrix();

    glDepthFunc(GL_LEQUAL);
    Math::Mat4 without_translation_view = view;
    without_translation_view.v[0].w = 0.0f;
    without_translation_view.v[1].w = 0.0f;
    without_translation_view.v[2].w = 0.0f;
    skybox_shader.setView(without_translation_view);
    skybox_shader.setProjection(perspective);
    skybox_shader.setTexture3D("uSkyboxTexture", 0, sky_cubemap);
    GFX::DrawGeometry(skybox_geo, &skybox_shader);
    glDepthFunc(GL_LESS);

    model = Math::Mat4::Identity();
    model = Math::Mat4::Scale(model, 5);
    model = Math::Mat4::Rotate(model, Math::Quat::FromEuler(90, 90, 0));
    model = Math::Mat4::Translate(model, 0, 5, 0);
    church_shader.setModel(model);
    church_shader.setView(view);
    church_shader.setProjection(perspective);
    GFX::DrawGeometry(church, &church_shader);

    GFX::ClearTelemetry();
}

void error_callback(int error, const char *msg) {
    std::string s;
    s = " [" + std::to_string(error) + "] " + msg + '\n';
    LOG_ERROR("%s\n", s.c_str());
}

GLFWwindow* GLFW_INIT() {
    RUNTIME_ASSERT_MSG(glfwInit(), "Failed to init glfw\n");
    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(app.WIDTH, app.HEIGHT, "LearnOpenGL", nullptr, nullptr);
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
    glfwSetInputMode(window, GLFW_CURSOR, app.mouse_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

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
    Input::GLFW_BIND_MOUSE_MOVE_CALLBACK(mouse);
    Input::CreateProfile(MASTER_PROFILE, cbMasterProfile);
    Input::CreateProfile(MOVEMENT_PROFILE, cbMovementProfile);
    
    skybox_shader = ShaderNoMaterial({"../../Scenes/ChurchScene/Shaders/Skybox/skybox.vert", "../../Scenes/ChurchScene/Shaders/Skybox/skybox.frag"});
    church_shader = ShaderMaterial({"../../Scenes/ChurchScene/Shaders/Model/model.vert", "../../Scenes/ChurchScene/Shaders/Model/model.frag"});

    DS::Vector<const char*> cubemap_faces = {
        "../../Assets/Textures/sky_skybox/right.jpg",
        "../../Assets/Textures/sky_skybox/left.jpg",
        "../../Assets/Textures/sky_skybox/top.jpg",
        "../../Assets/Textures/sky_skybox/bottom.jpg",
        "../../Assets/Textures/sky_skybox/front.jpg",
        "../../Assets/Textures/sky_skybox/back.jpg",
    };

    skybox_geo = GFX::Geometry::Cube();
    sky_cubemap = Texture::LoadCubeMap(cubemap_faces);

    church = GFX::Geometry::Model("../../Assets/Models/church.glb");

    float previous = 0;
    float timer = 2.0f;
	while (!glfwWindowShouldClose(window)) {
        float current = glfwGetTime();
        app.dt = current - previous;
        previous = current;

        app.accumulator += app.dt;

        if (timer == 0) {
            timer = 2.0f;
            LOG_DEBUG("FPS: %d\n", (int)(1.0f / app.dt));
        } else {
            timer = Math::MoveToward(timer, 0, app.dt);
        }

        Input::Poll();

        update();
		display();

		glfwPollEvents();
        glfwSwapBuffers(window);
	}

	return 0;
}