#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef HW2_TARGET
#define HW2_TARGET

class Target {
public:
    Target(GLuint programID, GLuint verticesBufferID, GLuint colorsBufferID, size_t verticesCount);

    void set_coordinates(glm::vec3 coordinates);

    void set_texture(GLuint textureID);

    bool is_close_to_point(glm::vec3 point);

    void draw(const glm::mat4 &MVP);

private:
    void calculate_rotation_matrix(glm::mat4 &rotation_matrix);

private:
    static void enable_attribute_buffer(size_t attribute_id, size_t attribute_size, GLuint buffer);

private:
    glm::vec3 coordinates_;

    GLint texture_location_ = 0;
    GLint rotate_location_ = 0;
    GLint matrix_location_ = 0;

    GLuint vertex_buffer_id_ = 0;
    GLuint uv_buffer_id_ = 0;
    GLuint program_id_ = 0;
    GLuint texture_id_ = 0;

    GLfloat current_spin_angle_ = 0.0f;
    GLuint vertex_count_ = 0;

private:
    constexpr static GLfloat kAngleStep = 0.002f;

    constexpr static auto kPi = glm::pi<GLfloat>();
};

#endif //HW2_TARGET
