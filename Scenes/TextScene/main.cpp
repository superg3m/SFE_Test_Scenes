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

    glEnable(GL_MULTISAMPLE);  
    GFX::SetDepthTest(true);
    GFX::SetStencilTest(true);

    return window;
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key > 255 || action != GLFW_PRESS) return;

    LOG_DEBUG("Key: (%c, %d) | IS_ALPHA: %s\n", (unsigned char)key, key, CHAR_IS_ALPHA(key) ? "TRUE" : "FALSE");
}

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;

void my_stbtt_initfont() {
    size_t file_size = 0;
    Error error = Error::SUCCESS;
    const char* font_path = "C:/Windows/Fonts/arial.ttf";
    u8* ttf_data = Platform::ReadEntireFile(font_path, file_size, error);
    RUNTIME_ASSERT_MSG(error == Error::SUCCESS, "%s\n", getErrorString(error));           

    stbtt_fontinfo font;
    stbtt_InitFont(&font, ttf_data, stbtt_GetFontOffsetForIndex(ttf_data, 0));

    int width = 0;
    int height = 0;
    int codepoint = 'N';
    int pixel_scale = 128;
    int x_offset = 0;
    int y_offset = 0;
    u8* mono_bitmap = stbtt_GetCodepointBitmap(&font, 0, stbtt_ScaleForPixelHeight(&font, pixel_scale), codepoint, &width, &height, &x_offset, &y_offset);

    u32* rgba_bitmap = (u32*)Memory::Malloc(sizeof(u32) * width * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int mono_sample_index = (((height - 1) - y) * width) + x;
            u8 alpha = mono_bitmap[mono_sample_index];
            rgba_bitmap[(y * width) + x] = (alpha << 24)|(alpha << 16)|(alpha << 8)|(alpha << 0);
        }
    }

    // can cache glyph here and make opengl texture from memory
    this->texture = Texture::LoadFromMemory((u8*)rgba_bitmap, width, height, 4, true);

    stbtt_FreeBitmap(mono_bitmap, nullptr);
    Memory::Free(ttf_data);
    Memory::Free(rgba_bitmap);

    stbtt_aligned_quad q;
    stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
    glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0);
    glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
    glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1);
    glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1);

    Math::Vec3 tr = Math::Vec3(+0.05f, +0.05f, +0.0f);
    Math::Vec3 br = Math::Vec3(+0.05f, -0.05f, +0.0f);
    Math::Vec3 bl = Math::Vec3(-0.05f, -0.05f, +0.0f);
    Math::Vec3 tl = Math::Vec3(-0.05f, +0.05f, +0.0f);

    this->quad = GFX::Geometry::Quad(tr, br, bl, tl);
}


struct AppState {
    Random::Seed seed = Random::GenerateSeed(451);
    Memory::GeneralAllocator allocator = Memory::GeneralAllocator();
    GLFWwindow* window;

    float dt = 0;
    float WIDTH = 900;
    float HEIGHT = 900;
    float accumulator = 0;

    GFX::Geometry quad;
    ShaderNoMaterial text_shader; // probably need to make this Shader3DNoMaterial and Shader2DNoMaterial or something like this
    Texture texture;

    AppState() {
        Memory::bindAllocator(&this->allocator);
        this->window = GLFW_INIT(this->WIDTH, this->HEIGHT);

        Math::Vec3 tr = Math::Vec3(+0.05f, +0.05f, +0.0f);
        Math::Vec3 br = Math::Vec3(+0.05f, -0.05f, +0.0f);
        Math::Vec3 bl = Math::Vec3(-0.05f, -0.05f, +0.0f);
        Math::Vec3 tl = Math::Vec3(-0.05f, +0.05f, +0.0f);

        this->quad = GFX::Geometry::Quad(tr, br, bl, tl);
        this->text_shader = ShaderNoMaterial({"../../Scenes/TextScene/Shaders/Text/text.vert", "../../Scenes/TextScene/Shaders/Text/text.frag"});
    }
};

AppState app;

void render() {
    glClearColor(0.2f, 0.2f, 0.2f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    app.text_shader.setTexture2D("uTexture", 0, app.texture);
    app.quad.draw(&app.text_shader);
}

int main(int argc, char** argv) {
    Input::Init();
    if (!Input::GLFW_SETUP(app.window)) {
        LOG_ERROR("Failed to setup GLFW\n");
        glfwTerminate();
        exit(-1);
    }

    Input::GLFW_BIND_KEY_CALLBACK(keyboard);

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
