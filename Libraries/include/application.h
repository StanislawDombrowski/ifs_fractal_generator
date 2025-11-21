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

class Application
{
private:
    /* data */
public:
    Renderer renderer;
    IFS state;

    Application(/* args */);
    ~Application();

    GLFWwindow* Init();
    void Run();
};

Application::Application(/* args */)
{
}

Application::~Application()
{
}


