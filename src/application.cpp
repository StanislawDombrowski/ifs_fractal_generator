#include "application.h"

Application::Application(/* args */)
{
}

Application::~Application()
{
}



GLFWwindow* Application::Init(){
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
    if (window == nullptr){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        
    }

    glfwMakeContextCurrent(window);
    
    gladLoadGL();
    glViewport(0, 0, 800, 600);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Set ImGui style
    ImGui::StyleColorsDark();

    const char* glsl_version = "#version 440"; // Match your shader version
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    
    return window;
}

void Application::Run(GLFWwindow *window){
    while (!glfwWindowShouldClose(window)){
        input.handleEvents(window, ifs, renderer, ui);
        ui.renderUI(ifs, renderer, input, window);
        renderer.render(window, input);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}