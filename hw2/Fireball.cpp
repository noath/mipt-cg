#include "Fireball.hpp"

Fireball::Fireball(GLuint programID, GLuint vertexbuffer, GLuint uvbuffer, size_t vertex_count) :
        program_id_(programID),
        vertex_buffer_id_(vertexbuffer),
        uv_buffer_id_(uvbuffer),
        vertex_count_(vertex_count) {
    matrix_location_ = glGetUniformLocation(programID, "MVP");
    rotate_location_ = glGetUniformLocation(programID, "rotation_matrix");
    texture_location_ = glGetUniformLocation(programID, "myTextureSampler");
}

void Fireball::set_move_direction(glm::vec3 move_direction) {
    move_direction_ = glm::normalize(move_direction);
}

void Fireball::set_coordinates(glm::vec3 coordinates) {
    coordinates_ = coordinates;
}

void Fireball::set_texture_id(GLuint textureID) {
    texture_id = textureID;
}

glm::vec3 Fireball::get_current_position() {
    return move_direction_ * dist_to_launch_point_ + coordinates_;
}

bool Fireball::calculate_rotation_matrix(glm::mat4 &rotation_matrix) {
    dist_to_launch_point_ += kDistStep;
    current_spin_angle_ += kAngleStep;

    // Normalize angle, so that -pi <= angle <= pi
    GLint n = current_spin_angle_ / kPi;
    current_spin_angle_ -= 2 * kPi * n;

    // Scale model, so that distant objects look smaller
    glm::vec3 current_position = get_current_position();
    GLfloat scale = 1 / glm::length(current_position);
    if (scale < 0.1) {
        return false;
    }
    rotation_matrix = glm::translate(glm::mat4(), current_position) *
                      glm::rotate(glm::mat4(1.0f), current_spin_angle_, glm::vec3(0, 0, 1)) *
                      glm::scale(glm::mat4(), glm::vec3(scale, scale, scale));
    return true;
}

void Fireball::enable_attribute_buffer(size_t attribute_id, size_t attribute_size, GLuint buffer) {
    glEnableVertexAttribArray(attribute_id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(attribute_id, attribute_size, GL_FLOAT, GL_FALSE, 0, nullptr);
}

bool Fireball::draw(const glm::mat4 &MVP) {
    glm::mat4 rotation_matrix;
    bool is_bullet_visible = calculate_rotation_matrix(rotation_matrix);
    if (!is_bullet_visible) {
        return false;
    }
    // Use our shader
    glUseProgram(program_id_);

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    glUniformMatrix4fv(matrix_location_, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(rotate_location_, 1, GL_FALSE, &rotation_matrix[0][0]);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(texture_location_, 0);

    // vertices
    enable_attribute_buffer(0, 3, vertex_buffer_id_);
    // colors
    enable_attribute_buffer(1, 2, uv_buffer_id_);

    // Draw triangles
    glDrawArrays(GL_TRIANGLES, 0, vertex_count_);

    // Disable attribute arrays
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    return true;
}