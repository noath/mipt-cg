#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef HW2_BULLET
#define HW2_BULLET

class Fireball {
public:
    Fireball(GLuint programID, GLuint vertexbuffer, GLuint uvbuffer, size_t vertex_count);

    void set_coordinates(glm::vec3 coordinates);

    void set_move_direction(glm::vec3 move_direction);

    void set_texture_id(GLuint textureID);

    glm::vec3 get_current_position();

    bool draw(const glm::mat4 &MVP);

private:
    bool calculate_rotation_matrix(glm::mat4 &rotation_matrix);

    void enable_attribute_buffer(size_t attribute_id, size_t attribute_size, GLuint buffer);

private:
    glm::vec3 move_direction_;
    glm::vec3 coordinates_;

    GLint rotate_location_ = 0;
    GLint matrix_location_ = 0;
    GLint texture_location_ = 0;

    GLuint vertex_buffer_id_ = 0;
    GLuint uv_buffer_id_ = 0;
    GLuint program_id_ = 0;
    GLuint texture_id = 0;

    GLfloat current_spin_angle_ = 0.0f;
    GLfloat dist_to_launch_point_ = 1.5f;
    size_t vertex_count_ = 0;

private:
    constexpr static GLfloat kDistStep = 0.07f;
    constexpr static GLfloat kAngleStep = 0.07f;
    constexpr static auto kPi = glm::pi<GLfloat>();
};

#endif //HW2_BULLET
