#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Input;
class IFS;
class Renderer;

class UI{
private:

public:
    ImGuiWindowFlags window_flags;
    bool built;
    ImGuiID dockspace_id;

    ImGuiID dock_right;
    ImGuiID dock_left;
    ImGuiViewport* vp;

    UI();
    ~UI();

    ImGuiViewport* initUI();
    void drawFrame(IFS &ifs, Renderer &renderer, Input &input, GLFWwindow* window);

    void BeginFrame();
    void EndFrame();

    void renderUI(IFS &ifs, Renderer &renderer, Input &input, GLFWwindow* window);
};