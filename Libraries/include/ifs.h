#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <map>

#include "shaders.h"

struct GenerationData {
    std::vector<glm::dvec4> points;
    std::vector<glm::dmat4> transforms;

    unsigned int vao;
    unsigned int vbo;
    unsigned int tfo;
    int point_count;
};

struct ifs_state{
    int num_of_transforms;
    int draw_index;
    std::vector<GenerationData> history;
};

class IFS
{
private:

public:
    ifs_state state;
    GenerationData data;

    unsigned int generationShader;

    IFS();
    ~IFS();

    unsigned int initShaders(std::vector<std::string> sourcesPath, std::vector<GLenum> types);

    void init_genesis(ifs_state &state, std::vector<glm::dvec4>& initial_points);
    void generate_points(int depth, ifs_state &state, unsigned int program);

    std::vector<glm::dmat4> init_transforms(std::string matrix_source, ifs_state &state);

    void update_IFS_data(ifs_state &state, std::vector<glm::dmat4> transforms, unsigned int program);
};



