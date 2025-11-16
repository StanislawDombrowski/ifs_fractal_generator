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

void processCameraInput(GLFWwindow *window, double dt, input_variables &inputs, ifs_state &state, Camera &camera)
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

void processCameraInput3D(GLFWwindow* window, const ImGuiIO& io, double dt, Camera& cam) {
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
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