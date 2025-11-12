#include "input.h"


void processInput(GLFWwindow *window, input_variables &variables)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        variables.num_generations++;
        variables.should_generate = true;     
}
