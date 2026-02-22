#include <SFE/sfe.hpp>

#define MASTER_PROFILE "master"
#define MOVEMENT_PROFILE "movement"

struct ApplicationState {
    float dt = 0;
    float accumulator = 0;
    float WIDTH = 900;
    float HEIGHT = 900;

    bool show_editor = true;

    int frame_count = 0;
    int rolling_fps = 0;
    bool mouse_captured = true;
    float bg_color[4] = {0.25f, 0.25f, 0.25f, 1.0f};

    Camera camera = Camera(0, 1, 10);

    Texture fire_texture;
};

struct EditorState {
    ImGuizmo::OPERATION gizmo_operation = ImGuizmo::TRANSLATE;
    bool show_demo_window = false;
};

ApplicationState app;
EditorState editor;

void cbMasterProfile() {
    GLFWwindow* window = (GLFWwindow*)Input::glfw_window_instance;
    const bool SHIFT = Input::GetKey(Input::KEY_SHIFT, Input::PRESSED|Input::DOWN);

    if (Input::GetKeyPressed(Input::KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }

    if (Input::GetKeyPressed(Input::KEY_R)) {}

    if (Input::GetKeyPressed(Input::KEY_PERIOD)) {
        app.show_editor = !app.show_editor;
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

void render_scene() {
    Math::Mat4 model = Math::Mat4::Identity();
    Math::Mat4 perspective = GFX::GetProjectionMatrix3D(app.WIDTH, app.HEIGHT, app.camera.zoom);
    Math::Mat4 view = app.camera.getViewMatrix();
}

void render_gui() {
    ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
            if (ImGui::BeginTabBar("Editor")) {
                if (ImGui::BeginTabItem("Telemetry")) {
                    ImGui::Text("FPS: %d", app.rolling_fps);
                    ImGui::SameLine();
                    ImGui::Checkbox("Demo Window", &editor.show_demo_window);

                    if (ImGui::RadioButton("Translate", editor.gizmo_operation == ImGuizmo::TRANSLATE)) {
                        editor.gizmo_operation = ImGuizmo::TRANSLATE;
                    }
                    
                    ImGui::SameLine();
                    if (ImGui::RadioButton("Rotate", editor.gizmo_operation == ImGuizmo::ROTATE)) {
                        editor.gizmo_operation = ImGuizmo::ROTATE;
                    }

                    ImGui::SameLine();
                    if (ImGui::RadioButton("Scale", editor.gizmo_operation == ImGuizmo::SCALE)) {
                        editor.gizmo_operation = ImGuizmo::SCALE;
                    }

                    ImGui::ColorEdit4("Background Color", app.bg_color);

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Model Loader")) {
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Texture Viewer")) {
                    if (ImGui::Button("Load Texture")) {
                        // Use NFD
                    }

                    for (int i = 0; i < 2; i++) {
                        ImGui::Image((ImTextureID)0, ImVec2(64, 64));
                        ImGui::SameLine();
                    }

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGuiWindowFlags flags = (
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | 
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground
            );

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2((float)app.WIDTH, (float)app.HEIGHT));
            ImGui::Begin("GizmoOverlay", nullptr, flags);
                /*
                if (picked_entity.reference) {
                    ImGuizmo::SetOrthographic(false);
                    ImGuizmo::SetDrawlist();
                    ImGuizmo::SetRect(0, 0, (float)app.WIDTH, (float)app.HEIGHT);
                    GM_Matrix4 viewT = view.transpose();
                    GM_Matrix4 projT = projection.transpose();
                    GM_Matrix4 newTransformT = picked_entity.reference->transform.getMatrix().transpose();
                    bool is_manipulated = ImGuizmo::Manipulate(&viewT.v[0].x, &projT.v[0].x, editor.gizmo_operation, ImGuizmo::WORLD, &newTransformT.v[0].x);
                    if (is_manipulated) {
                        GM_Matrix4 newTransform = newTransformT.transpose();
                        picked_entity.reference->transform.setMatrix(newTransform);
                    }
                }
                */
            ImGui::End();

            if (editor.show_demo_window) {
                ImGui::ShowDemoWindow(&editor.show_demo_window);
            }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void render() {
    glClearColor(app.bg_color[0], app.bg_color[1], app.bg_color[2], app.bg_color[3]);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    render_scene();
    if (app.show_editor) {
        render_gui();
    }
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

bool IMGUI_INIT(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    bool success = ImGui_ImplGlfw_InitForOpenGL(window, true);
    if (!success) {
        LOG_ERROR("[IMGUI ERROR]: ImGui_ImplGlfw_InitForOpenGL() Failed\n");
    }

    success = ImGui_ImplOpenGL3_Init("#version 330");
    if (!success) {
        LOG_ERROR("[IMGUI ERROR]: ImGui_ImplOpenGL3_Init(#version 330) Failed\n");
    }

    return success;
}

void BEGIN_FRAME() {
    static float previous = 0;
    float current = glfwGetTime();
    app.dt = current - previous;
    previous = current;

    app.accumulator += app.dt;

    #define DELTA_TIME_BUFFER_SIZE 30
    static float delta_time_buffer[DELTA_TIME_BUFFER_SIZE] = {0};

    int next_dt_index = app.frame_count % DELTA_TIME_BUFFER_SIZE;
    delta_time_buffer[next_dt_index] = app.dt;

    if (next_dt_index == (DELTA_TIME_BUFFER_SIZE - 1)) {
        app.rolling_fps = (int)(1.0f / Math::Average(delta_time_buffer, DELTA_TIME_BUFFER_SIZE));
    }
}

void END_FRAME(GLFWwindow* window) {
    glfwPollEvents();
    glfwSwapBuffers(window);

    app.frame_count += 1;
    GFX::ClearTelemetry();
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

    if (!IMGUI_INIT(window)) {
        return -1;
    }

    app.fire_texture = Texture::LoadFromFile("../../Assets/Textures/fire.jpg");

	while (!glfwWindowShouldClose(window)) {
        BEGIN_FRAME();

        Input::Poll();
        update();
		render();

        END_FRAME(window);
	}

	return 0;
}