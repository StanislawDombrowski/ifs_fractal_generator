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