#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders.h"
#include "ifs.h"

#include <vector>
#include <array>
#include <string>
#include <map>

using glm::dmat4;
using glm::dvec4;

class Renderer {
public:
    unsigned int shader;
    IFS ifs;

    Renderer();
    ~Renderer();

    unsigned int initShaders(std::vector<std::string> sourcesPath, std::vector<GLenum> types);

    std::array<unsigned int, 2> initVBOs();
    std::array<unsigned int, 2> initVAOs(const int MAX_POINTS, std::array<unsigned int, 2> VBOs);
    std::array<unsigned int, 2> initTFOs(std::array<unsigned int, 2> VBOs);

    void fillVBO(unsigned int VBO, std::vector<glm::dvec4> points);
};

Renderer::Renderer(){

}

Renderer::~Renderer(){
    
}
