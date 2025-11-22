#include "UI.h"
#include "input.h"    
#include "ifs.h"      
#include "renderer.h" 

UI::UI(){
    window_flags = ImGuiWindowFlags_NoDocking |
                    ImGuiWindowFlags_NoTitleBar |
                    ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                    ImGuiWindowFlags_NoNavFocus |
                    ImGuiWindowFlags_NoBackground;

    built = false;
    dock_right = 0;
    dock_left = 0;
}

UI::~UI(){

}

ImGuiViewport* UI::initUI(){

        ImGuiViewport* vp = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0)); 
        ImGui::Begin("DockSpace Host", nullptr, window_flags);
        
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);

        return vp;
}

void UI::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    vp = initUI();
}

void UI::EndFrame() {
    // 3. Finalize the frame logic
    ImGui::Render();

    // 4. Draw the UI data to the screen
    // Note: We don't clear the screen here (glClear), 
    // because your Renderer class usually does that before this is called.
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::drawFrame(IFS &ifs, Renderer &renderer, Input &input, GLFWwindow* window){
        if (!built)
        {
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

            int generation = ifs.state.history.size();
        
            if (!ifs.state.history.empty()) {
                ImGui::Text("Generations: %d", (int)ifs.state.history.size());
                // .back() gets the last element
                ImGui::Text("Total Points: %d", ifs.state.history.back().point_count);
            } else {
                ImGui::Text("Generations: 0");
                ImGui::Text("Total Points: 0");
            }
            ImGui::Separator();
            if (ImGui::Button("Generate Next Level")) {
                generation++;
                ifs.generate_points(1, ifs.state, ifs.generationShader);
            }
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            if (ImGui::Button(input.camera.perspective ? "Switch to Ortho 2D" : "Switch to Perspective 3D")) {
            input.camera.perspective = !input.camera.perspective;

            if (input.camera.perspective) {
                // Initialize orbit state from current pos/front/up
                input.camera.target = input.camera.cameraPos + input.camera.cameraFront * 5.0; // heuristic
                glm::dvec3 dir = glm::normalize(input.camera.target - input.camera.cameraPos);
                input.camera.pitch_deg = glm::degrees(asin(dir.y));
                input.camera.yaw_deg = glm::degrees(atan2(dir.x, -dir.z)); // consistent with -Z forward
                input.camera.distance = glm::length(input.camera.target - input.camera.cameraPos);
                if (input.camera.distance < 0.1) input.camera.distance = 0.1;
            } else {
                // For ortho mode, align cameraFront to -Z and keep pos
                input.camera.cameraFront = glm::dvec3(0, 0, -1);
                input.camera.cameraUp = glm::dvec3(1e-16, 1, 0); // avoid exact 0 cross issues
            }
        }

        if (input.camera.perspective) {
            ImGui::SliderFloat("FOV (deg)", (float*)&input.camera.fov_deg, 20.0f, 90.0f);
            ImGui::SliderFloat("Distance", (float*)&input.camera.distance, 0.1f, 200.0f);
            ImGui::SliderFloat("Yaw", (float*)&input.camera.yaw_deg, -180.0f, 180.0f);
            ImGui::SliderFloat("Pitch", (float*)&input.camera.pitch_deg, -89.0f, 89.0f);
            }
            ImGui::End();
            
}

void UI::renderUI(IFS &ifs, Renderer &renderer, Input &input, GLFWwindow* window)
{
    initUI();
    drawFrame(ifs, renderer, input, window);

    glDisable(GL_BLEND); 
    glDisable(GL_DEPTH_TEST);
}