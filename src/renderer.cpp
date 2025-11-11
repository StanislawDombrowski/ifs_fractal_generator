#include "renderer.h"

Renderer::Renderer() {
    // Constructor implementation (if needed)
}

Renderer::~Renderer() {
    // Destructor implementation (if needed)
}

void Renderer::clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::drawArrays(unsigned int mode, int first, int count) {
    glDrawArrays(mode, first, count);
}

unsigned int Renderer::initVBO() {
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    return VBO;
}

unsigned int Renderer::initVAO() {
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    return VAO;
}

unsigned int Renderer::fillVBO(unsigned int VBO, std::vector<glm::vec4>& data) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec4), data.data(), GL_STATIC_DRAW);
    return VBO;
}

unsigned int Renderer::appendTransformToVBO(unsigned int VBO, std::vector<glm::mat4>& data) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, data.size() * sizeof(glm::mat4), data.data()); // Example: overwrite from start
    return VBO;
}

void Renderer::InitVertexAttribPointer(unsigned int index, int size, unsigned int type, bool normalized, size_t stride, const void* pointer) {
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, type, normalized ? GL_TRUE : GL_FALSE, static_cast<GLsizei>(stride), pointer);
}

pointRenderer::pointRenderer() {
    // Constructor implementation (if needed)
}

pointRenderer::~pointRenderer() {
    // Destructor implementation (if needed)
}

void pointRenderer::setPointSize(float size) {
    glPointSize(size);
}

void pointRenderer::renderPoints(unsigned int VBO, std::vector<glm::vec3>& points) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Fill buffer with point data
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    drawArrays(GL_POINTS, 0, static_cast<int>(points.size()));
}