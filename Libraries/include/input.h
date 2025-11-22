#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include "imgui.h"
#include <glm/glm.hpp>

#include <algorithm>
#include "ifs.h"
#include "UI.h"
#include "renderer.h"

struct Camera{
    // Camera setup
    glm::dvec3 cameraPos   = glm::dvec3(0.5, 0.25, 1.0); // Start centered on the fractal
    glm::dvec3 cameraFront = glm::dvec3(0.0, 0.0, -1.0);
    glm::dvec3 cameraUp    = glm::dvec3(0.0, 1.0, 0.0);

    double orthoSize = 4.0f;
    double BASE_PAN_SPEED = 1.5f;

    bool perspective = false;

    // perspective params
    double fov_deg = 45.0;
    double nearPlane = 0.01;
    double farPlane = 1000.0;

    // orbit controller state
    glm::dvec3 target{0.0, 0.0, 0.0};
    double distance = 5.0;  // radius from target
    double yaw_deg = -90.0; // yaw=0 looks +X, -90 looks -Z
    double pitch_deg = 0.0; // clamp to (-89, 89)

    // mouse state
    bool rotating = false;  // RMB
    bool panning = false;   // LMB
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;

    // sensitivities
    double rotate_sens = 0.2;   // deg per pixel
    double pan_sens = 0.0015;   // world units per pixel scaled by distance
    double dolly_sens = 1.05;   // multiplicative distance zoom

    glm::dmat4 projection, view;
};

struct input_variables
{
    bool should_generate = false;
};

class Input
{
private:

public:
    input_variables vars;
    Camera camera;

    Input();
    ~Input();

    void processInput(GLFWwindow *window, input_variables &variables);
    void processCameraInput(GLFWwindow *window, double dt, input_variables &inputs, ifs_state &state, Camera &camera);
    void processCameraInput3D(GLFWwindow* window, const ImGuiIO& io, double dt, Camera& cam);

    void processCamera(GLFWwindow* window, UI ui, Renderer renderer);

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    void setCallbacks(GLFWwindow* window);

    void handleEvents(GLFWwindow *window, IFS ifs, Renderer renderer, UI ui);
};



