#include "renderer.h"

Renderer::Renderer() {
    // Constructor implementation (if needed)
}

Renderer::~Renderer() {
    // Destructor implementation (if needed)
}

std::array<unsigned int, 2> Renderer::initVBOs() {
    std::array<unsigned int, 2> VBOs;
    glGenBuffers(2, VBOs.data());
    return VBOs;
}

std::array<unsigned int, 2> Renderer::initVAOs(const int MAX_POINTS, std::array<unsigned int, 2> VBOs) {
    std::array<unsigned int, 2> VAOs;
    glGenVertexArrays(2, VAOs.data());
    for (size_t i = 0; i < 2; i++)
    {
        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec4) * MAX_POINTS, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribLPointer(0, 4, GL_DOUBLE, 0, (void*)0);
    }
    return VAOs;
}

std::array<unsigned int, 2> Renderer::initTFOs(std::array<unsigned int, 2> VBOs){
    std::array<unsigned int, 2> tfos;
    glGenTransformFeedbacks(2, tfos.data());


    // Set up transform feedback to capture data into the VBOs
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfos[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBOs[0]);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfos[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBOs[1]);

    // Unbind the tfos
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    return tfos;
}

void Renderer::fillVBO(unsigned int VBO, std::vector<glm::dvec4> points){
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(glm::dvec4), points.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}



