#include "input.h"

Input::Input()
{

}

Input::~Input()
{

}

void Input::processInput(GLFWwindow *window, input_variables &variables)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

}

void Input::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Input::processCameraInput(GLFWwindow *window, double dt, input_variables &inputs, ifs_state &state, Camera &camera)
{
    double cameraSpeed = camera.BASE_PAN_SPEED *  camera.orthoSize * dt;
    

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
        camera.orthoSize *= zoomSensitivity;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        camera.orthoSize /= zoomSensitivity;
}

void Input::processCameraInput3D(GLFWwindow* window, const ImGuiIO& io, double dt, Camera& cam) {
    if (!cam.perspective) return;

    // If UI is capturing mouse, don't move camera
    if (io.WantCaptureMouse) {
        cam.rotating = cam.panning = false;
        return;
    }

    // Mouse buttons
    bool rmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    bool lmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    // On button press edge, start drag
    if (rmb && !cam.rotating) { cam.rotating = true; cam.lastMouseX = mx; cam.lastMouseY = my; }
    if (!rmb) cam.rotating = false;

    if (lmb && !cam.panning) { cam.panning = true; cam.lastMouseX = mx; cam.lastMouseY = my; }
    if (!lmb) cam.panning = false;

    double dx = mx - cam.lastMouseX;
    double dy = my - cam.lastMouseY;
    cam.lastMouseX = mx;
    cam.lastMouseY = my;

    // Build current camera axes from yaw/pitch
    double cy = cos(glm::radians(cam.yaw_deg));
    double sy = sin(glm::radians(cam.yaw_deg));
    double cp = cos(glm::radians(cam.pitch_deg));
    double sp = sin(glm::radians(cam.pitch_deg));

    // Forward points from camera toward target
    glm::dvec3 forward = glm::normalize(glm::dvec3(sy * cp, sp, -cy * cp));
    glm::dvec3 right   = glm::normalize(glm::cross(forward, glm::dvec3(0, 1, 0)));
    glm::dvec3 up      = glm::normalize(glm::cross(right, forward));

    // Rotate with RMB: change yaw/pitch
    if (cam.rotating) {
        cam.yaw_deg   += cam.rotate_sens * dx;
        cam.pitch_deg -= cam.rotate_sens * dy;
        cam.pitch_deg = std::clamp(cam.pitch_deg, -89.0, 89.0);
    }

    // Pan with LMB: move the target in the view plane
    if (cam.panning) {
        double scale = cam.pan_sens * cam.distance;
        cam.target -= right * (dx * scale);
        cam.target += up    * (dy * scale);
    }

    // Optional keyboard dolly (W/S) in perspective mode
    double dollySpeed = 2.0 * dt * std::max(0.1, cam.distance);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cam.distance = std::max(0.05, cam.distance - dollySpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cam.distance = std::min(1000.0, cam.distance + dollySpeed);

    // Mouse wheel dolly (read from your scroll callback if you prefer)
    // Example inline read:
    // Prefer using your existing scroll callback to modify cam.distance or cam.fov

    // Recompute basis after updates
    cy = cos(glm::radians(cam.yaw_deg));
    sy = sin(glm::radians(cam.yaw_deg));
    cp = cos(glm::radians(cam.pitch_deg));
    sp = sin(glm::radians(cam.pitch_deg));
    forward = glm::normalize(glm::dvec3(sy * cp, sp, -cy * cp));
    right   = glm::normalize(glm::cross(forward, glm::dvec3(0, 1, 0)));
    up      = glm::normalize(glm::cross(right, forward));

    // Update camera position/front/up
    cam.cameraPos   = cam.target - forward * cam.distance;
    cam.cameraFront = forward;
    cam.cameraUp    = up;
}

void Input::processCamera(GLFWwindow* window, UI ui, Renderer renderer)
{
    int fb_w = 0, fb_h = 0;
    glfwGetFramebufferSize(window, &fb_w, &fb_h);

    float sx = (ui.vp->Size.x > 0.0f) ? (float)fb_w / ui.vp->Size.x : 1.0f;
    float sy = (ui.vp->Size.y > 0.0f) ? (float)fb_h / ui.vp->Size.y : 1.0f;

    ImGuiDockNode* left_node = ImGui::DockBuilderGetNode(ui.dock_left);

    // Fallback: whole work area if node is not ready
    ImVec2 pos  = left_node ? left_node->Pos  : ui.vp->WorkPos;
    ImVec2 size = left_node ? left_node->Size : ui.vp->WorkSize;

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
    if (camera.perspective) {
        double ar = (double)aspectRatio;
        camera.projection = glm::perspective(glm::radians(camera.fov_deg), ar, camera.nearPlane, camera.farPlane);
    } else {
        camera.projection = glm::ortho(-camera.orthoSize * aspectRatio,
                             camera.orthoSize * aspectRatio,
                            -camera.orthoSize, camera.orthoSize,
                            -1.0, 1.0);
    }
    camera.view = glm::lookAt(camera.cameraPos,
                            camera.cameraPos + camera.cameraFront,
                            camera.cameraUp);
                        processCameraInput(window, renderer.deltaTime, vars, renderer.ifs.state, camera);
}

void Input::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    const float zoomSensitivity = 0.90f; // Must be < 1.0 for zooming in

    if (yoffset > 0) {
        // Zoom in by multiplying by a factor < 1
        camera->orthoSize *= zoomSensitivity;
    }
    else if (yoffset < 0) {
        // Zoom out by dividing by the same factor
        camera->orthoSize /= zoomSensitivity;
    }

    // We no longer need to clamp the minimum zoom, as it will never reach zero.
    // But it's still wise to clamp the maximum to prevent the user from getting lost.
    if (camera->orthoSize > 100.0f) {
        camera->orthoSize = 100.0f;
    }
}

void Input::setCallbacks(GLFWwindow* window)
{
    
    glfwSetWindowUserPointer(window, &camera);

    glfwSetFramebufferSizeCallback(window, Input::framebuffer_size_callback);     
    glfwSetScrollCallback(window, Input::scroll_callback);
}

void Input::handleEvents(GLFWwindow *window, IFS ifs, Renderer renderer, UI ui)
{
    glfwPollEvents();
    processInput(window, vars);
    if (!camera.perspective) {
        processCameraInput(window, renderer.deltaTime, vars, ifs.state, camera);
    } else {
        processCameraInput3D(window, ImGui::GetIO(), renderer.deltaTime, camera);
    }
    processCamera(window, ui, renderer);
}
