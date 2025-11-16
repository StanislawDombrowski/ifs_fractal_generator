#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "input.h"
#include "shaders.h"
#include "renderer.h"
#include "ifs.h"

int main(){

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
    if (window == nullptr){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
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

    input_variables variableses;
    Camera camera;
    ifs_state state;
    Renderer renderer;

    glfwSetWindowUserPointer(window, &camera);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);     
    glfwSetScrollCallback(window, scroll_callback);

    // Points and matrix data:
    std::vector<glm::dvec4> points = {
        glm::dvec4(0.0, 0.0, 0.0, 1.0)
    };
    std::vector<glm::dmat4> transforms = init_transforms("../../transformations/pyramid.txt", state);


    // Load and compile shaders for generation
    const std::string vertexShaderSource = readShader("../../src/shaders/vertex_shader.vert");
    const std::string geometryShaderSource = readShader("../../src/shaders/geometry.geom");
    const std::string generationFragmentSource = readShader("../../src/shaders/generation_frag.frag");

    const std::string fragmentShaderSource = readShader("../../src/shaders/fragment_shader.frag");

    // Shaders for drawing
    const std::string drawVertexShaderSource = readShader("../../src/shaders/draw_vertex_shader.vert");

    // Compile vertex shader
    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int drawVertexShader = compileShader(drawVertexShaderSource, GL_VERTEX_SHADER);
    // Compile geometry shader
    unsigned int geometryShader = compileShader(geometryShaderSource, GL_GEOMETRY_SHADER);
    // Compile fragment shader
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    unsigned int generationFragment = compileShader(generationFragmentSource, GL_FRAGMENT_SHADER);

    // Link shaders to create shader program
    unsigned int generateProgram = createShaderProgram({vertexShader, geometryShader, generationFragment});
    unsigned int drawProgram = createShaderProgram({drawVertexShader, fragmentShader});
    // Individual shader cleanup
    // glDeleteShader(vertexShader);
    // glDeleteShader(fragmentShader);
    // glDeleteShader(geometryShader);

    // ------------------ Generation of points part of program -------------------------

    // Generate buffers for ping-pong technique
    const int MAX_POINTS = 50000000;
    
    std::array<unsigned int, 2> VBOs = renderer.initVBOs();
    std::array<unsigned int, 2> VAOs = renderer.initVAOs(MAX_POINTS, VBOs);
    
    // Upload initial poinst to the buffer
    renderer.fillVBO(VBOs[0], points);

    state.VAOs = VAOs;

    // Generate the Transform Feedback Objects 
    std::array<unsigned int, 2> tfos = renderer.initTFOs(VBOs);

    // Tell the geometry shader which data to capture
    const char* feedbackVaryings[] = {"out_Pos"}; // Name of the variable to capture
    glTransformFeedbackVaryings(generateProgram, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(generateProgram); // Relink the program required


    // Using the generation program
    glUseProgram(generateProgram);

    state.num_points = points.size();
    state.num_generations = 1;
    state.last_index = 0;

    update_IFS_data(state, transforms, generateProgram);

    //state.last_index = generate_points(state.num_generations, state, tfos, generateProgram, 0);
    // ------------------ Final Drawing Step ------------------------


    // Unbind to keep a clean state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    double lastFrame = 0.0;
    double deltaTime = 0.0;
    double fpsEMA = 0.0;      // smoothed fps
    const double fpsAlpha = 0.1; // smoothing factor in [0,1]
    
    double fpsAccumTime = 0.0;
    int fpsFrames = 0;

    glPointSize(1.0f);

    glLinkProgram(drawProgram);
    glUseProgram(drawProgram); // Use the program for drawing

    bool key_was_pressed = false;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoBackground;

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0)); 

        ImGui::Begin("DockSpace Host", nullptr, window_flags);

        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);

        static bool built = false;
        static ImGuiID dock_right = 0, dock_left = 0;
        if (!built) {
            built = true;
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, vp->Size);

            ImGuiID root = dockspace_id;
            ImGui::DockBuilderSplitNode(root, ImGuiDir_Right, 0.28f, &dock_right, &dock_left);
            ImGui::DockBuilderDockWindow("Settings", dock_right);
            // Do NOT dock a Viewport window. Leave dock_left empty: that is the passthrough area.
            ImGui::DockBuilderFinish(dockspace_id);
            }   
            ImGui::End();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(3);

            ImGui::Begin("Settings", nullptr,
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoTitleBar);

            ImGui::Text("Generations: %d", state.num_generations);
            ImGui::Text("Total Points: %d", state.num_points);
            ImGui::Separator();
            if (ImGui::Button("Generate Next Level")) {
                state.num_generations++;
                int new_read_idx = generate_points(1, state, tfos.data(), generateProgram, state.last_index);
                state.last_index = new_read_idx;
            }
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            if (ImGui::Button(camera.perspective ? "Switch to Ortho 2D" : "Switch to Perspective 3D")) {
            camera.perspective = !camera.perspective;

            if (camera.perspective) {
                // Initialize orbit state from current pos/front/up
                camera.target = camera.cameraPos + camera.cameraFront * 5.0; // heuristic
                glm::dvec3 dir = glm::normalize(camera.target - camera.cameraPos);
                camera.pitch_deg = glm::degrees(asin(dir.y));
                camera.yaw_deg = glm::degrees(atan2(dir.x, -dir.z)); // consistent with -Z forward
                camera.distance = glm::length(camera.target - camera.cameraPos);
                if (camera.distance < 0.1) camera.distance = 0.1;
            } else {
                // For ortho mode, align cameraFront to -Z and keep pos
                camera.cameraFront = glm::dvec3(0, 0, -1);
                camera.cameraUp = glm::dvec3(1e-16, 1, 0); // avoid exact 0 cross issues
            }
        }

        if (camera.perspective) {
            ImGui::SliderFloat("FOV (deg)", (float*)&camera.fov_deg, 20.0f, 90.0f);
            ImGui::SliderFloat("Distance", (float*)&camera.distance, 0.1f, 200.0f);
            ImGui::SliderFloat("Yaw", (float*)&camera.yaw_deg, -180.0f, 180.0f);
            ImGui::SliderFloat("Pitch", (float*)&camera.pitch_deg, -89.0f, 89.0f);
}
            ImGui::End();
            
            int fb_w = 0, fb_h = 0;
            glfwGetFramebufferSize(window, &fb_w, &fb_h);

            float sx = (vp->Size.x > 0.0f) ? (float)fb_w / vp->Size.x : 1.0f;
            float sy = (vp->Size.y > 0.0f) ? (float)fb_h / vp->Size.y : 1.0f;

            ImGuiDockNode* left_node = ImGui::DockBuilderGetNode(dock_left);

            // Fallback: whole work area if node is not ready
            ImVec2 pos  = left_node ? left_node->Pos  : vp->WorkPos;
            ImVec2 size = left_node ? left_node->Size : vp->WorkSize;

            // Convert to framebuffer pixels
            int vx = (int)std::floor(pos.x * sx);
            int vy_top = (int)std::floor(pos.y * sy);
            int vw = (int)std::floor(size.x * sx);
            int vh = (int)std::floor(size.y * sy);
            int vy = fb_h - (vy_top + vh); // flip Y

            // 1) Clear full framebuffer once
            glDisable(GL_SCISSOR_TEST);
            glViewport(0, 0, fb_w, fb_h);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // 2) Clear only the left region (one time), then disable scissor
            glEnable(GL_SCISSOR_TEST);
            glScissor(vx, vy, vw, vh);
            glClearColor(58.0f / 255.0f, 61.0f / 255.0f, 59.0f / 255.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_SCISSOR_TEST);

            // 3) Draw into left region
            glViewport(vx, vy, vw, vh);
            glDisable(GL_DEPTH_TEST); // optional, safer for point cloud
            // Use aspect from vw/vh
            float aspectRatio = (vh > 0) ? (float)vw / (float)vh : 1.0f;
            glm::dmat4 projection, view;
            if (camera.perspective) {
                double ar = (double)aspectRatio;
                projection = glm::perspective(glm::radians(camera.fov_deg), ar, camera.nearPlane, camera.farPlane);
            } else {
                projection = glm::ortho(-camera.orthoSize * aspectRatio,
                                     camera.orthoSize * aspectRatio,
                                    -camera.orthoSize, camera.orthoSize,
                                    -1.0, 1.0);
            }

            view = glm::lookAt(camera.cameraPos,
                            camera.cameraPos + camera.cameraFront,
                            camera.cameraUp);
                        processCameraInput(window, deltaTime, variableses, state, camera);

            glUseProgram(drawProgram);
            glUniformMatrix4dv(glGetUniformLocation(drawProgram, "projection"),
                            1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4dv(glGetUniformLocation(drawProgram, "view"),
                            1, GL_FALSE, glm::value_ptr(view));

            // Draw your points
            glBindVertexArray(VAOs[state.last_index]);
            glDrawArrays(GL_POINTS, 0, state.num_points);

            // /* ----------- DEBUG INFO ---------------*/

            double now = glfwGetTime();
            deltaTime = now - lastFrame;
            lastFrame = now;

            processInput(window, variableses);
            if (!camera.perspective) {
                processCameraInput(window, deltaTime, variableses, state, camera);
            } else {
                processCameraInput3D(window, ImGui::GetIO(), deltaTime, camera);
            }


            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
            glfwSwapBuffers(window);
        }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
