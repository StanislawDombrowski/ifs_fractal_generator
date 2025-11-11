#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>

class Renderer {
public:
    Renderer();
    ~Renderer();

    void clear(float r, float g, float b, float a);
    void drawArrays(unsigned int mode, int first, int count);

    unsigned int initVBO();
    unsigned int initVAO();

    // Fill VBO with point data
    unsigned int fillVBO(unsigned int VBO, std::vector<glm::vec4>& data);
    // Append ifs data to an existing VBO
    unsigned int appendTransformToVBO(unsigned int VBO, std::vector<glm::mat4>& data);

    void InitVertexAttribPointer(unsigned int index, int size, unsigned int type, bool normalized, size_t stride, const void* pointer);

};



class pointRenderer : public Renderer {
public:
    pointRenderer();
    ~pointRenderer();

    void setPointSize(float size);
    void renderPoints(unsigned int VBO, std::vector<glm::vec3>& points);

};

class triangleRenderer : public Renderer {
public:
    triangleRenderer();
    ~triangleRenderer();

    void renderTriangles(unsigned int VBO, std::vector<glm::vec3>& vertices);
};