#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <random>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

GLFWwindow *window;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>

#include "Target.hpp"
#include "Fireball.hpp"

class Game {
public:
    Game() {
        srand (static_cast <unsigned> (time(0)));
        loaded = true;
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            loaded = false;
        }

        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(1024, 768, "Shoot the target", nullptr, nullptr);
        if (nullptr == window) {
            std::cerr << "Failed to open GLFW window" << std::endl;
            glfwTerminate();
            loaded = false;
        }
        glfwMakeContextCurrent(window);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            getchar();
            glfwTerminate();
            loaded = false;
        }

        // Ensure we can capture the escape key being pressed below
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
        // Hide the mouse and enable unlimited mouvement
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // Grey layout
        glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);

        // Create and compile our GLSL program from the shaders
        targetProgramID = LoadShaders("shaders/VertexShader.glsl",
                                      "shaders/FragmentShader.glsl");
        fireballProgramID = LoadShaders("shaders/VertexShader.glsl",
                                        "shaders/FragmentShader.glsl");

        // load textures
        lavaTexture = loadBMP_custom("assets/lava.bmp");
        goldTexture = loadBMP_custom("assets/gold.bmp");

        std::vector<glm::vec3> normals; // we won't use it, so it's local
        // Read our .obj file
        bool op1_res = loadOBJ("assets/target.obj", target_vertices, target_uv, normals);
        if (op1_res) {
            // Load it into a VBO
            glGenBuffers(1, &target_vertexbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, target_vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, target_vertices.size() * sizeof(glm::vec3), &target_vertices[0],
                         GL_STATIC_DRAW);

            glGenBuffers(1, &target_uvbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, target_uvbuffer);
            glBufferData(GL_ARRAY_BUFFER, target_uv.size() * sizeof(glm::vec2), &target_uv[0],
                         GL_STATIC_DRAW);
        }
        else {
            std::cerr << "Failed to load .obj" << std::endl;
            loaded = false;
        }
        // Read our .obj file
        bool op2_res = loadOBJ("assets/ball.obj", fireball_vertices, fireball_uv, normals);
        if (op2_res) {
            // Load it into a VBO
            glGenBuffers(1, &fireball_vertexbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, fireball_vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, fireball_vertices.size() * sizeof(glm::vec3), &fireball_vertices[0], GL_STATIC_DRAW);

            glGenBuffers(1, &fireball_uvbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, fireball_uvbuffer);
            glBufferData(GL_ARRAY_BUFFER, fireball_uv.size() * sizeof(glm::vec2), &fireball_uv[0], GL_STATIC_DRAW);
        }
        else {
            std::cerr << "Failed to load .obj" << std::endl;
            loaded = false;
        }
    }

    ~Game() {
        // Cleanup VBO
        glDeleteVertexArrays(1, &VertexArrayID);
        glDeleteProgram(targetProgramID);
        glDeleteProgram(fireballProgramID);

        glDeleteBuffers(1, &fireball_vertexbuffer);
        glDeleteBuffers(1, &fireball_uvbuffer);
        glDeleteBuffers(1, &target_vertexbuffer);
        glDeleteBuffers(1, &target_uvbuffer);

        glfwTerminate();
    }

    int run() const {
        if (!loaded) {
            return -1;
        }

        std::vector<Target> targets;
        std::vector<Fireball> fireballs;

        int total_shoots = 0;
        int total_hits = 0;

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        const glm::mat4 Projection = glm::perspective(glm::radians(45.0f),
                                                      4.0f / 3.0f,
                                                      0.1f,
                                                      100.0f);

        // Set the mouse at the center of the screen
        glfwPollEvents();
        glfwSetCursorPos(window, 1024.0 / 2, 768.0 / 2);

        int mouseState = GLFW_RELEASE;

        double last_add_time = glfwGetTime();

        do {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Compute the MVP matrix from keyboard and mouse input
            computeMatricesFromInputs();
            glm::mat4 ProjectionMatrix = getProjectionMatrix();
            glm::mat4 ViewMatrix = getViewMatrix();
            glm::mat4 ModelMatrix = glm::mat4(1.0);
            glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

            double curr_time = glfwGetTime();

            if (curr_time - last_add_time > 1 && targets.size() < 16) {
                spawn_target(targets);
                last_add_time = curr_time;
            }

            int currMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if (mouseState == GLFW_RELEASE && currMouseState == GLFW_PRESS) {
                total_shoots += 1;
                spawn_fireball(fireballs);
            }
            mouseState = currMouseState;

            // Colliding proccessing
            for (size_t i = 0; i < fireballs.size(); ++i) {
                bool collided = false;
                glm::vec3 pos = fireballs[i].get_current_position();
                for (size_t j = 0; j < targets.size(); ++j) {
                    if (targets[j].is_close_to_point(pos)) {
                        total_hits += 1;
                        collided = true;
                        targets.erase(targets.begin() + j);
                        --j;
                    }
                }
                if (collided) {
                    fireballs.erase(fireballs.begin() + i);
                    --i;
                }
            }

            // Drawing targets
            for (int i = 0; i < targets.size(); ++i) {
                targets[i].draw(MVP);
            }

            // Drawing fireballs
            for (int i = 0; i < fireballs.size(); ++i) {
                // Too far fireball case
                if (!fireballs[i].draw(MVP)) {
                    fireballs.erase(fireballs.begin() + i);
                    --i;
                }
            }

            // Swap buffers
            glfwSwapBuffers(window);
            glfwPollEvents();

        } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

        return 0;
    }
private:
    // Vertex array ID
    GLuint VertexArrayID;

    // Shaders ID
    GLuint targetProgramID;
    GLuint fireballProgramID;

    // Texture IDs
    GLuint lavaTexture;
    GLuint goldTexture;


    std::vector<glm::vec3> fireball_vertices;
    std::vector<glm::vec2> fireball_uv;
    GLuint fireball_vertexbuffer;
    GLuint fireball_uvbuffer;

    std::vector<glm::vec3> target_vertices;
    std::vector<glm::vec2> target_uv;
    GLuint target_vertexbuffer;
    GLuint target_uvbuffer;

    bool loaded;

    void spawn_target(std::vector<Target> &targets) const {
        auto new_target = Target(targetProgramID, target_vertexbuffer, target_uvbuffer, target_vertices.size() * 3);
        new_target.set_texture(goldTexture);

        const double r = 2.0 + static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (10.0 - 2.0)));
        const double phi = static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (2 * glm::pi<float>() - 0.0)));
        const double psi = static_cast<double>(rand()) / (static_cast<double>(RAND_MAX / (2 * glm::pi<float>() - 0.0)));
        glm::vec3 pos = glm::vec3(
                cos(phi) * sin(psi) * r,
                sin(phi) * r,
                cos(phi) * cos(psi) * r
        );

        new_target.set_coordinates(pos);

        targets.push_back(new_target);
    }

    void spawn_fireball(std::vector<Fireball> &fireballs) const {
        auto new_fireball = Fireball(fireballProgramID, fireball_vertexbuffer, fireball_uvbuffer, fireball_vertices.size() * 3);
        new_fireball.set_texture_id(lavaTexture);
        new_fireball.set_coordinates(getPosition());
        new_fireball.set_move_direction(getDirection());

        fireballs.push_back(new_fireball);
    }


};

int main() {
    auto game = Game();
    int op_code = game.run();
    return op_code;
}
