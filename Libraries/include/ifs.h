#pragma once

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>

struct ifs_state{
    int num_points;
    int num_generations;
    int num_of_transforms;
    std::array<unsigned int, 2> VAOs;
    unsigned int last_index;
};

std::vector<glm::dmat4> init_transforms(std::string matrix_source, ifs_state &state);

int generate_points(int depth, ifs_state &state, unsigned int* tfos, unsigned int program, int start_read_idx);

void update_IFS_data(ifs_state &state, std::vector<glm::dmat4> transforms, unsigned int program);