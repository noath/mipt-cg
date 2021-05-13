#include "Target.hpp"

Target::Target(GLuint programID, GLuint verticesBufferID, GLuint colorsBufferID, size_t verticesCount) :
        program_id_(programID),
        vertex_buffer_id_(verticesBufferID),
        uv_buffer_id_(colorsBufferID),
        vertex_count_(verticesCount) {
    matrix_location_ = glGetUniformLocation(programID, "MVP");
    rotate_location_ = glGetUniformLocation(programID, "rotation_matrix");
    texture_location_ = glGetUniformLocation(programID, "myTextureSampler");
}

void Target::set_coordinates(glm::vec3 coordinates) {
    coordinates_ = coordinates;
}

void Target::set_texture(GLuint textureID) {
    texture_id_ = textureID;
}

bool Target::is_close_to_point(glm::vec3 point) {
    return glm::distance(point, coordinates_) < 0.3;
}

void Target::calculate_rotation_matrix(glm::mat4 &rotation_matrix) {
    current_spin_angle_ += kAngleStep;

    // Normalize angle, so that -pi <= angle <= pi
    GLint n = current_spin_angle_ / kPi;
    current_spin_angle_ -= 2 * kPi * n;

    // Scale model, so that distant objects look smaller
    GLfloat scale = 1 / glm::length(coordinates_);
    rotation_matrix = glm::translate(glm::mat4(), coordinates_) *
                      glm::rotate(glm::mat4(1.0f), current_spin_angle_, glm::vec3(0, 0, 1)) *
                      glm::scale(glm::mat4(), glm::vec3(scale, scale, scale));
}

void Target::enable_attribute_buffer(size_t attribute_id, size_t attribute_size, GLuint buffer) {
    glEnableVertexAttribArray(attribute_id);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(attribute_id, attribute_size, GL_FLOAT, GL_FALSE, 0, nullptr);
}

void Target::draw(const glm::mat4 &MVP) {
    // Use our shader
    glUseProgram(program_id_);

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    glm::mat4 rotation_matrix;
    calculate_rotation_matrix(rotation_matrix);
    glUniformMatrix4fv(matrix_location_, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(rotate_location_, 1, GL_FALSE, &rotation_matrix[0][0]);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id_);

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
}