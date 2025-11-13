#include "input.h"


void processInput(GLFWwindow *window, input_variables &variables)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

void processCameraInput(GLFWwindow *window, double dt, input_variables &inputs, ifs_state &state, Camera &camera, double cameraSpeed, double &orthoSize)
{
    cameraSpeed *= orthoSize * dt;
    

    // Pan Up
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.cameraPos += cameraSpeed * camera.cameraUp;
    // Pan Down
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.cameraPos -= cameraSpeed * camera.cameraUp;
    // Pan Left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.cameraPos -= glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * cameraSpeed;
    // Pan Right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.cameraPos += glm::normalize(glm::cross(camera.cameraFront, camera.cameraUp)) * cameraSpeed;

    const float zoomSensitivity = 0.90f;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        orthoSize *= zoomSensitivity;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        orthoSize /= zoomSensitivity;
}