#pragma once

#include <GLFW/glfw3.h>

struct input_variables
{
    bool should_generate = false;
    int num_generations = 1;
};

void processInput(GLFWwindow *window, input_variables &variables);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);