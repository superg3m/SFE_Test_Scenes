#include <SFE/sfe.hpp>

ShaderNoMaterial cloud_shader;
ShaderNoMaterial terrain_shader;
ShaderNoMaterial uniform_shader;

GFX::Geometry terrain;
GFX::Geometry clouds;
GFX::Geometry light;

bool emit = false;
Texture diffuse;
Texture height;
Texture cloud;

#define MASTER_PROFILE "master"
#define MOVEMENT_PROFILE "movement"
#define LIGHT_PROFILE "light"

// This should be a grouping of state
bool smooth_camera = false;
Camera camera = Camera(0, 1, 10);
bool mouse_captured = true;
float dt = 0;
float accumulator = 0;
float WIDTH = 900;
float HEIGHT = 900;
int height_boost = 0;
bool render_shading = true;
bool render_normals = false;

Math::Vec3 light_position = Math::Vec3(0, 10, 0);
Math::Vec3 camera_target = Math::Vec3(0, 20, 0);
int camera_point_index = 0;
Math::Vec3 camera_points[4];
const float TERRAIN_SCALE = 0.25;

bool movement_profile_active = true; 

Math::Vec3 vec3MoveTowardModified(Math::Vec3 a, Math::Vec3 b, float delta) {
    Math::Vec3 ret = Math::Vec3(0);
    ret.x = Math::MoveToward(a.x, b.x, delta);
    ret.y = Math::MoveToward(a.y, b.y, 0.2f * delta);
    ret.z = Math::MoveToward(a.z, b.z, delta);
    
    return ret;
}

void cbMasterProfile() {
    GLFWwindow* window = (GLFWwindow*)Input::glfw_window_instance;
    const bool SHIFT = Input::GetKey(Input::KEY_SHIFT, Input::PRESSED|Input::DOWN);

    if (Input::GetKeyPressed(Input::KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }

    if (Input::GetKeyPressed(Input::KEY_R)) {
        cloud_shader.compile();
        terrain_shader.compile();
        uniform_shader.compile();
    }

    if (Input::GetKeyPressed(Input::KEY_K)) {
        smooth_camera = !smooth_camera;
        Input::ToggleProfile(MOVEMENT_PROFILE, movement_profile_active && !smooth_camera);
    }

    if (Input::GetKeyPressed(Input::KEY_0)) {
        emit = !emit;
    }

    if (SHIFT && Input::GetKeyPressed(Input::KEY_W)) {
        movement_profile_active = !movement_profile_active;
        Input::ToggleProfile(MOVEMENT_PROFILE, movement_profile_active && !smooth_camera);
        Input::ToggleProfile(LIGHT_PROFILE, !movement_profile_active);
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

    if (Input::GetKeyPressed(Input::KEY_0)) {
        render_shading = !render_shading;
    }

    if (Input::GetKeyPressed(Input::KEY_N)) {
        render_normals = !render_normals;
    }

    if (Input::GetKey(Input::KEY_UP, Input::PRESSED|Input::DOWN)) {
        height_boost += 2;
    }

    if (Input::GetKey(Input::KEY_DOWN, Input::PRESSED|Input::DOWN)) {
        height_boost -= 2;
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

void cbLightMovementProfile() {
    if (Input::GetKey(Input::KEY_SPACE, Input::PRESSED|Input::DOWN)) {
        light_position.y += 1;
    }

    if (Input::GetKey(Input::KEY_CTRL, Input::PRESSED|Input::DOWN)) {
        light_position.y -= 1;
    }

    if (Input::GetKey(Input::KEY_W, Input::PRESSED|Input::DOWN)) {
        light_position.z -= 1;
    }

    if (Input::GetKey(Input::KEY_A, Input::PRESSED|Input::DOWN)) {
        light_position.x -= 1;
    }

    if (Input::GetKey(Input::KEY_S, Input::PRESSED|Input::DOWN)) {
        light_position.z += 1;
    }

    if (Input::GetKey(Input::KEY_D, Input::PRESSED|Input::DOWN)) {
        light_position.x += 1;
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

    if (mouse_captured) {
        camera.processMouseMovement(xoffset, yoffset);
    }
}

void update() {
    if (smooth_camera) {
        camera.position = vec3MoveTowardModified(camera.position, camera_points[camera_point_index], camera.movement_speed * dt);
        camera.lookat(camera_target);
        if (camera.position == camera_points[camera_point_index]) {
            camera_point_index = (camera_point_index + 1) % 4;
        }
    }
}

void display() {
    glClearColor(0.2f, 0.2f, 0.2f, 0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    Math::Mat4 perspective = GFX::GetProjectionMatrix3D(WIDTH, HEIGHT, camera.zoom);
    Math::Mat4 view = camera.getViewMatrix();

    cloud_shader.setProjection(perspective);
    terrain_shader.setProjection(perspective);
    uniform_shader.setProjection(perspective);

    cloud_shader.setView(view);
    terrain_shader.setView(view);
    uniform_shader.setView(view);

    Math::Mat4 model = Math::Mat4::Identity();
    // model = Math::Mat4::Scale(model, 5);
    // model = Math::Mat4::Rotate(model, Math::Quat::FromEuler(90, 90, 0));
    // model = Math::Mat4::Translate(model, 0, 5, 0);
    // model_shader.setModel(model);
    // model_shader.setEmissiveMaterial(emit);
    // church.draw(&model_shader);


    model = Math::Mat4::Identity();
    model = Math::Mat4::Translate(model, light_position);
    uniform_shader.setModel(model);
    GFX::DrawGeometry(light, &uniform_shader);


    model = Math::Mat4::Identity();
    model = Math::Mat4::Scale(model, TERRAIN_SCALE);
    terrain_shader.setModel(model);
    terrain_shader.setFloat("uWidth", diffuse.width);
    terrain_shader.setFloat("uHeight", diffuse.height);
    terrain_shader.setInt("uHeightBoost", height_boost);
    terrain_shader.setTexture2D("uColorTexture", 0, diffuse);
    terrain_shader.setTexture2D("uHeightTexture", 1, height);
    terrain_shader.setVec3("uLightPosition", light_position);
    terrain_shader.setVec3("uLightPosition", light_position);
    terrain_shader.setBool("uRenderShading", render_shading);
    terrain_shader.setBool("uRenderNormals", render_normals);
    GFX::DrawGeometry(terrain, &terrain_shader);

    model = Math::Mat4::Identity();
    model = Math::Mat4::Scale(model, diffuse.width * TERRAIN_SCALE, diffuse.height * TERRAIN_SCALE, 1);
    model = Math::Mat4::Rotate(model, Math::Quat::FromEuler(-90, 0, 0));
    model = Math::Mat4::Translate(model, 0, 40, 0);
    cloud_shader.setModel(model);
    cloud_shader.setTexture2D("uCloudTexture", 0, cloud);
    cloud_shader.setFloat("uOffsetX", cosf(0.05 * accumulator));
    cloud_shader.setFloat("uOffsetY", sinf(0.05 * accumulator));
    GFX::SetBlending(true);
    GFX::DrawGeometry(clouds, &cloud_shader);
    GFX::SetBlending(false);

    // LOG_WARN("Draw Call Count: %d\n", GFX::GetDrawCallCount());

    GFX::ClearTelemetry();
}

void error_callback( int error, const char *msg ) {
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
    glfwSetInputMode(window, GLFW_CURSOR, mouse_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

    glEnable(GL_MULTISAMPLE);
    GFX::SetDepthTest(true);
    GFX::SetStencilTest(true);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_FRAMEBUFFER_SRGB);

    return window;
}

void init_geometry() {
    diffuse = Texture::LoadFromFile("../../Assets/Textures/world_color_map.png");
    height = Texture::LoadFromFile("../../Assets/Textures/world_height_map.png");
    cloud = Texture::LoadFromFile("../../Assets/Textures/world_cloud_map.png");

    terrain = GFX::Geometry::Quad(diffuse.width, diffuse.height);
    camera_points[0] = Math::Vec3{-((diffuse.width * TERRAIN_SCALE) / 2.0f), 60, (diffuse.height * TERRAIN_SCALE) / 2.0f};
    camera_points[1] = Math::Vec3{-((diffuse.width * TERRAIN_SCALE) / 2.0f), 30, -(diffuse.height * TERRAIN_SCALE) / 2.0f};
    camera_points[2] = Math::Vec3{((diffuse.width * TERRAIN_SCALE) / 2.0f), 60, -(diffuse.height * TERRAIN_SCALE) / 2.0f};
    camera_points[3] = Math::Vec3{((diffuse.width * TERRAIN_SCALE) / 2.0f), 30, (diffuse.height * TERRAIN_SCALE) / 2.0f};

    light = GFX::Geometry::Cube();
    clouds = GFX::Geometry::Quad();
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
    Input::CreateProfile(LIGHT_PROFILE, cbLightMovementProfile, false);
    
    cloud_shader = ShaderNoMaterial({"../../Scenes/TerrainScene/Shaders/Cloud/cloud.vert", "../../Scenes/TerrainScene/Shaders/Cloud/cloud.frag"});
    terrain_shader = ShaderNoMaterial({"../../Scenes/TerrainScene/Shaders/Terrain/terrain.vert", "../../Scenes/TerrainScene/Shaders/Terrain/terrain.frag"});
    uniform_shader = ShaderNoMaterial({"../../Scenes/TerrainScene/Shaders/Uniform/uniform.vert", "../../Scenes/TerrainScene/Shaders/Uniform/uniform.frag"});

    init_geometry();

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
		display();

		glfwPollEvents();
        glfwSwapBuffers(window);
	}

	exit(EXIT_SUCCESS);
}

/*
Math::Vec3 getCirclePoint(Math::Vec3 center, float radius, float theta) {
    float rad = DEGREES_TO_RAD(theta);

    float offset_x = cos(rad) * radius;
    float offset_z = sin(rad) * radius;

    return Math::Vec3(center.x + offset_x, center.y, center.z + offset_z);
}


void draw_circle() {
    Math::Vec3 CENTER = Math::Vec3(0);
    const float RADIUS = 1.0f;
    const int SEGMENT_COUNT = 360;
    const float DELTA_THETA = 1 / SEGMENT_COUNT;

    for (int i = 0; i < SEGMENT_COUNT; i++) { 
        Math::Vec3 square_point = getSquarePoint(CENTER, DELTA_THETA * i, RADIUS);
        Math::Vec3 circle_point = getCirclePoint(CENTER, DELTA_THETA * i, RADIUS);
        uv.x = ((DELTA_THETA * i) / SEGMENT_COUNT); // you can divide by 4 to make it on each the sides
        uv.y = 0 to 1; // if you are botton triangle you are 0.0f, if you are top triangle you are 1.0f

        // derive the quad segmenet from center
        Math::Vec3 p0; // BL (Bottom Left)
        Math::Vec3 p1; // TL
        Math::Vec3 p2; // BR
        Math::Vec3 p3; // TR
        Math::Vec3 p4; // TL
        Math::Vec3 p5; // BR

        Math::Vec3 quad_normal = Math::Vec3::Cross(p1 - p0, p2 - p0);

        // Math::Vec3 closest = Math::Vec3::Closest(square_point, circle_point);

        Math::Vec2 normalized_camera_front = Math::Vec2(camera.front.x, camera.front.z).normalize();
        Math::Vec2 normalized_segment_normal = Math::Vec2(normal.x, normal.z).normalize();
        float camera_normal_dot = Math::Vec2::Dot(normalized_camera_front, normalized_segment_normal);
        bool front_facing = (camera_normal_dot >= -1.0f) && (camera_normal_dot <= -0.8f);
        // draw_slice(p0, )
    }
}
*/