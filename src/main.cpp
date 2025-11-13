#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

using glm::mat4;
using glm::vec3;
using glm::radians;
using glm::translate;
using glm::scale;
using glm::rotate; 
using glm::dmat4; // double-precision matrix
using glm::dvec4; // double-precision vector

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "input.h"
#include "shaders.h"
#include "renderer.h"
#include "ifs.h"


double orthoSize = 4.0f;
const double BASE_PAN_SPEED = 2.5f;



void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    const float zoomSensitivity = 0.90f; // Must be < 1.0 for zooming in

    if (yoffset > 0) {
        // Zoom in by multiplying by a factor < 1
        orthoSize *= zoomSensitivity;
    }
    else if (yoffset < 0) {
        // Zoom out by dividing by the same factor
        orthoSize /= zoomSensitivity;
    }

    // We no longer need to clamp the minimum zoom, as it will never reach zero.
    // But it's still wise to clamp the maximum to prevent the user from getting lost.
    if (orthoSize > 100.0f) {
        orthoSize = 100.0f;
    }
}

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

    glfwSetScrollCallback(window, scroll_callback);
    //glfwSetKeyCallback(window, key_callback);

    input_variables variableses;
    Camera camera;
    ifs_state state;
    Renderer renderer;

    // Points and matrix data:
    std::vector<glm::dvec4> points = {
        glm::dvec4(0.0, 0.0, 0.0, 1.0)
    };
    std::vector<glm::dmat4> transforms = init_transforms("../../transformations/sierpinski.txt", state);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 

    // Load and compile shaders for generation
    const std::string vertexShaderSource = readShader("../../src/shaders/vertex_shader.vert");
    const std::string geometryShaderSource = readShader("../../src/shaders/geometry.geom");
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

    // Link shaders to create shader program
    unsigned int generateProgram = createShaderProgram({vertexShader, geometryShader, fragmentShader});
    unsigned int drawProgram = createShaderProgram({drawVertexShader, fragmentShader});
    // Individual shader cleanup
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(geometryShader);

    // ------------------ Generation of points part of program -------------------------

    // Generate buffers for ping-pong technique
    const int MAX_POINTS = 50000000;
    
    std::array<unsigned int, 2> VBOs = renderer.initVBOs();
    std::array<unsigned int, 2> VAOs = renderer.initVAOs(MAX_POINTS, VBOs);
    
    // Upload initial poinst to the buffer
    renderer.fillVBO(VBOs[0], points);

    state.VAOs = VAOs;

    // Generate the Transform Feedback Objects 
    unsigned int tfos[2];
    glGenTransformFeedbacks(2, tfos);

    // Set up transform feedback to capture data into the VBOs
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfos[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBOs[0]);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfos[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBOs[1]);

    // Unbind the tfos
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

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

    glUseProgram(drawProgram); // Use the program for drawing
    bool key_was_pressed = false;
    while (!glfwWindowShouldClose(window))
    {
        /* ----------- DEBUG INFO ---------------*/

        // Calculate the timings
        double now = glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;

        double fpsInstant = (deltaTime > 0.0) ? 1.0 / deltaTime : 0.0;
        if (fpsEMA == 0.0) fpsEMA = fpsInstant;
        else fpsEMA = fpsAlpha * fpsInstant + (1.0 - fpsAlpha) * fpsEMA;

        fpsAccumTime += deltaTime;
        fpsFrames += 1;

        if (fpsAccumTime >= 1.0) {
            double fpsAvg = fpsFrames / fpsAccumTime;
            std::cout << "OpenGL Window | FPS: " + std::to_string((int)fpsAvg) +
            " | smoothed: " + std::to_string((int)fpsEMA) +
            " | frame ms: " + std::to_string(deltaTime * 1000.0) << std::endl;

            fpsAccumTime = 0.0;
            fpsFrames = 0;
        }

        glClearColor(58.0f/255.0f, 61.0f/255.0f, 59.0f/255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(drawProgram);

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float aspectRatio = (float)width / (float)height;

        glm::dmat4 projection = glm::ortho(-orthoSize * aspectRatio, orthoSize * aspectRatio, -orthoSize, orthoSize, -1.0, 1.0);

        // View: creates a matrix that "looks" from cameraPos to a target
        glm::dmat4 view = glm::lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);

        glm::dmat4 identityMatrix = glm::dmat4(1.0);

        // 4. Upload Matrices to the Shader
        int projLoc = glGetUniformLocation(drawProgram, "projection");
        glUniformMatrix4dv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        int viewLoc = glGetUniformLocation(drawProgram, "view");
        glUniformMatrix4dv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Input handling
        processInput(window, variableses);
        processCameraInput(window, deltaTime, variableses, state, camera, BASE_PAN_SPEED, orthoSize);
        // Regenerate the points if needed
        bool g_key_is_down = (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS);
        if (g_key_is_down && !key_was_pressed)
        {
            // This block will now only execute ONCE per key press
            state.num_generations++;
            int new_read_idx = generate_points(1, state, tfos, generateProgram, state.last_index);
            state.last_index = new_read_idx;
            std::cout << "Generated points for generation: " << state.num_generations << std::endl;
            std::cout << "Recorded number of points: " << state.num_points << std::endl;
        }

        key_was_pressed = g_key_is_down;

        // Rendering
        glUseProgram(drawProgram);
        glDisable(GL_RASTERIZER_DISCARD);

        glBindVertexArray(VAOs[state.last_index]);
        glDrawArrays(GL_POINTS, 0, state.num_points);
        
        // Debug printing
        //std::cout << 1 - orthoSize << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
