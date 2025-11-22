#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "ifs.h"
#include "renderer.h"
#include "input.h"

#include <algorithm>


class UI{
private:

public:
    ImGuiWindowFlags window_flags;
    static bool built;
    ImGuiID dockspace_id;
    static ImGuiID dock_right;
    static ImGuiID dock_left;
    ImGuiViewport* vp;

    UI();
    ~UI();

    const ImGuiViewport* initUI();
    void drawFrame(IFS ifs, Renderer renderer, Input input, GLFWwindow* window);

    void renderUI(IFS ifs, Renderer renderer, Input input, GLFWwindow* window);
};