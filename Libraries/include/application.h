#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "ifs.h"
#include "renderer.h"
#include "input.h"
#include "UI.h"

class Application
{
private:
    /* data */
public:
    Renderer renderer;
    IFS ifs;
    Input input;
    UI ui;

    Application(/* args */);
    ~Application();

    GLFWwindow* Init();

    void Run(GLFWwindow *window);
};

