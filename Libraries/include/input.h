#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "ifs.h"

struct Camera{
    // Camera setup
    glm::dvec3 cameraPos   = glm::dvec3(0.5, 0.25, 1.0); // Start centered on the fractal
    glm::dvec3 cameraFront = glm::dvec3(0.0, 0.0, -1.0);
    glm::dvec3 cameraUp    = glm::dvec3(0.0, 1.0, 0.0);
};

struct input_variables
{
    bool should_generate = false;
};

void processInput(GLFWwindow *window, input_variables &variables);
void processCameraInput(GLFWwindow *window, double dt, input_variables &inputs, ifs_state &state, Camera &camera, double cameraSpeed, double &orthoSize);



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

