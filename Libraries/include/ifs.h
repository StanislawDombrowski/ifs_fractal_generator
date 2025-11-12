#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

int generate_points(int depth, unsigned int* VAOs, unsigned int* tfos, unsigned int program, int &num_points);

std::vector<glm::mat4> init_transforms(std::string matrix_source);