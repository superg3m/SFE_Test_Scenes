#include <SFE/sfe.hpp>

GLFWwindow* GLFW_INIT(int WIDTH, int HEIGHT) {
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

    glfwSwapInterval(1);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    glEnable(GL_MULTISAMPLE);  

    GFX::SetDepthTest(true);
    GFX::SetStencilTest(true);

    return window;
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key > 255 || action != GLFW_PRESS) return;

    LOG_DEBUG("Key: (%c, %d) | IS_ALPHA: %s\n", (unsigned char)key, key, CHAR_IS_ALPHA(key) ? "TRUE" : "FALSE");
}

struct AppState {
    Random::Seed seed = Random::GenerateSeed(451);
    Memory::GeneralAllocator allocator = Memory::GeneralAllocator();
    GLFWwindow* window;

    float dt = 0;
    float WIDTH = 900;
    float HEIGHT = 900;
    float accumulator = 0;

    ShaderNoMaterial text_shader;
    Texture texture;
    GFX::Font font;

    AppState() {
        Memory::bindAllocator(&this->allocator);
        this->window = GLFW_INIT(this->WIDTH, this->HEIGHT);

        #if defined(PLATFORM_WINDOWS)
            const char* font_path = "C:/Windows/Fonts/arial.ttf";
        #elif defined(PLATFORM_APPLE)
            const char* font_path = "/Library/Fonts/Arial Unicode.ttf";
        #endif

        this->font = GFX::Font::Create(font_path, 32,  ' ', '~');    

        this->text_shader = ShaderNoMaterial({"../../Scenes/TextScene/Shaders/Text/text.vert", "../../Scenes/TextScene/Shaders/Text/text.frag"});
    }
};

AppState app;

void cbMasterProfile() {
    GLFWwindow* window = (GLFWwindow*)Input::glfw_window_instance;
    // const bool SHIFT = Input::GetKey(Input::KEY_SHIFT, Input::PRESSED|Input::DOWN);

    if (Input::GetKeyPressed(Input::KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }

    if (Input::GetKeyPressed(Input::KEY_R)) {
        app.text_shader.compile();
    }

    if (Input::GetKeyPressed(Input::KEY_L)) {
        GFX::SetWireFrame(true);
    } else if (Input::GetKeyReleased(Input::KEY_L)) {
        GFX::SetWireFrame(false);
    }
}

void render() {
    glClearColor(0.2f, 0.2f, 0.2f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Math::Mat4 orthographic = GFX::GetProjectionMatrix2D(app.WIDTH, app.HEIGHT);

    app.text_shader.setProjection(orthographic);

    const char* text = "Wow this works\nthis doesn't work does it?";
    GFX::DrawText(app.font, text, 200, 200, &app.text_shader);
}

int main(int argc, char** argv) {
    Input::Init();
    if (!Input::GLFW_SETUP(app.window)) {
        LOG_ERROR("Failed to setup GLFW\n");
        glfwTerminate();
        exit(-1);
    }

    Input::GLFW_BIND_KEY_CALLBACK(keyboard);
    Input::CreateProfile("master", cbMasterProfile);

    float previous = 0;
    float timer = 2;
	while (!glfwWindowShouldClose(app.window)) {
        float current = glfwGetTime();
        float dt_for_fps = (current - previous);
        app.dt = (current - previous);
        previous = current;

        app.accumulator += app.dt;
        if (timer == 0) {
            timer = 2;
            LOG_DEBUG("FPS: %d\n", (int)(1.0f / dt_for_fps));
        } else {
            timer = Math::MoveToward(timer, 0, dt_for_fps);
        }
        
        Input::Poll();
        render();
        
        glfwSwapBuffers(app.window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
