// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>

using namespace glm;

#include <common/shader.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 MoveCameraAroundX(glm::mat4 Projection, glm::mat4 Model, float radius, float &angle, float delta = 1e-3f,
                            float x_bias = 0.0f, float y_bias = 0.0f, float z_bias = 0.0f) {
    auto camera_pos = glm::vec3(3);
    camera_pos[0] = 0.0f - x_bias; // X transformation (fixed)
    camera_pos[1] = radius * std::cos(angle) - y_bias; // Y transformation
    camera_pos[2] = radius * std::sin(angle) - z_bias; // Z transformation

    glm::mat4 CameraView = glm::lookAt(
            camera_pos, // moving in a circle
            glm::vec3(0, 0, 0), // always looks at the origin
            glm::vec3(0, 1, 0) // up
    );

    glm::mat4 MVP = Projection * CameraView * Model;
    angle += delta;

    return MVP;
}

glm::mat4 MoveCameraAroundY(glm::mat4 Projection, glm::mat4 Model, float radius, float &angle, float delta = 1e-3f,
                            float x_bias = 0.0f, float y_bias = 0.0f, float z_bias = 0.0f) {
    auto camera_pos = glm::vec3(3);
    camera_pos[0] = radius * std::cos(angle) - x_bias; // X transformation
    camera_pos[1] = 0.0f - y_bias; // Y transformation (fixed)
    camera_pos[2] = radius * std::sin(angle) - z_bias; // Z transformation

    glm::mat4 CameraView = glm::lookAt(
            camera_pos, // moving in a circle
            glm::vec3(0, 0, 0), // always looks at the origin
            glm::vec3(0, 1, 0) // up
    );

    glm::mat4 MVP = Projection * CameraView * Model;
    angle += delta;

    return MVP;
}

int main(void) {
    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Triangles", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr,
                "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID_1 = LoadShaders("TransformVertexShader.glsl", "SingleColorFragmentShader1.glsl");
    // Get a handle for our "MVP" uniform
    GLuint MatrixID_1 = glGetUniformLocation(programID_1, "MVP");

    // Create and compile our GLSL program from the shaders
    GLuint programID_2 = LoadShaders("TransformVertexShader.glsl", "SingleColorFragmentShader2.glsl");
    // Get a handle for our "MVP" uniform
    GLuint MatrixID_2 = glGetUniformLocation(programID_2, "MVP");

    // Projection matrix : 45° Field of View, 16:10 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 16.0f / 10.0f, 0.1f, 100.0f);

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);

    // Alpha channels
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    static const GLfloat g_vertex_buffer_data[] = {
            -0.6f, 0.5f, 0.0f,
            0.0f, 0.7f, 0.0f,
            0.7f, -0.2f, 0.0f,
            -0.5f, -0.3f, 0.0f,
            -0.3f, 0.6f, 0.0f,
            0.8f, 0.4f, 0.0f,
    };


    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    GLfloat current_angle = M_PI / 2;
    do {
        // Rotation of camera
        glm::mat4 MVP;
        if (current_angle < 2 * M_PI + M_PI / 2) {
            MVP = MoveCameraAroundY(Projection, Model, 2.0f, current_angle, M_PI / 360);
        } else if (current_angle < 4 * M_PI + M_PI / 2) {
            MVP = MoveCameraAroundX(Projection, Model, 2.0f, current_angle, M_PI / 360);
        } else {
            current_angle -= 4 * M_PI;
            continue;
        };
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);


        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void *) 0            // array buffer offset
        );

        //using first color shader
        glUseProgram(programID_1);
        //drawing first triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glUniformMatrix4fv(MatrixID_1, 1, GL_FALSE, &MVP[0][0]);

        //using second color shader
        glUseProgram(programID_2);
        //drawing second triangle
        glDrawArrays(GL_TRIANGLES, 3, 3);
        glUniformMatrix4fv(MatrixID_2, 1, GL_FALSE, &MVP[0][0]);

        glDisableVertexAttribArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID_1);
    glDeleteProgram(programID_2);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

