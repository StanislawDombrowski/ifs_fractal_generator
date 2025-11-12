#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

using glm::mat4;
using glm::vec3;
using glm::radians;
using glm::translate;
using glm::scale;
using glm::rotate;

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "input.h"
#include "shaders.h"
#include "renderer.h"
#include "ifs.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

// Camera setup
glm::vec3 cameraPos   = glm::vec3(0.5f, 0.25f, 1.0f); // Start centered on the fractal
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

// Timing for frame-rate independent movement
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float orthoSize = 4.0f;
const float BASE_PAN_SPEED = 2.5f;

void processCameraInput(GLFWwindow *window, float dt, input_variables &inputs, ifs_state &state)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float cameraSpeed = BASE_PAN_SPEED * orthoSize * dt;
    
    // Pan Up
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    // Pan Down
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;
    // Pan Left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    // Pan Right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    const float zoomSensitivity = 0.90f;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        orthoSize *= zoomSensitivity;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        orthoSize /= zoomSensitivity;
}

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

// void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
// {
//     if (key == GLFW_KEY_G && action == GLFW_PRESS)
//     {
//         variables.should_generate = true;
//         variables.num_generations++;
//     }
// }

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
    ifs_state state;
    variableses.should_generate = false;
    Renderer renderer;

    // Points and matrix data:
    std::vector<glm::vec4> points = {
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
    };

    std::vector<glm::mat4> transforms = init_transforms("/home/stanislaw/Documents/dev/ifs_fractal_generator/transformations/leaf.txt");

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 

    // Load and compile shaders for generation
    const std::string vertexShaderSource = readShader("/home/stanislaw/Documents/dev/ifs_fractal_generator/src/shaders/vertex_shader.vert");
    const std::string geometryShaderSource = readShader("/home/stanislaw/Documents/dev/ifs_fractal_generator/src/shaders/geometry.geom");
    const std::string fragmentShaderSource = readShader("/home/stanislaw/Documents/dev/ifs_fractal_generator/src/shaders/fragment_shader.frag");

    // Shaders for drawing
    const std::string drawVertexShaderSource = readShader("/home/stanislaw/Documents/dev/ifs_fractal_generator/src/shaders/draw_vertex_shader.vert");

    // Compile vertex shader
    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int drawVertexShader = compileShader(drawVertexShaderSource, GL_VERTEX_SHADER);
    // Compile geometry shader
    unsigned int geometryShader = compileShader(geometryShaderSource, GL_GEOMETRY_SHADER);
    // Compile fragment shader
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    // Link shaders to create shader program
    unsigned int generateProgram = glCreateProgram();
    glAttachShader(generateProgram, vertexShader);
    glAttachShader(generateProgram, geometryShader);
    unsigned int drawProgram = glCreateProgram();
    glAttachShader(drawProgram, drawVertexShader);
    glAttachShader(drawProgram, fragmentShader);
    glLinkProgram(drawProgram);
    // Individual shader cleanup
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(geometryShader);

    // ------------------ Generation of points part of program -------------------------

    // Generate buffers for ping-pong technique
    unsigned int VAOs[2];
    unsigned int VBOs[2];

    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);

    const int MAX_POINTS = 1000000;

    // Init buffer memory for all the data
    for (size_t i = 0; i < 2; i++)
    {
        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * MAX_POINTS, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    }
    
    // Upload initial poinst to the buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(glm::vec4), points.data());

    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL Error after initial upload: " << err << std::endl;
    }   

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

    GLint success;
    glGetProgramiv(generateProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(generateProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Using the generation program
    glUseProgram(generateProgram);

    // Set uniforms for the program
    int transformLoc = glGetUniformLocation(generateProgram, "transformations");
    if (transformLoc == -1) {
        std::cerr << "Could not find uniform 'transformations'!" << std::endl;
    } else {
        glUniformMatrix4fv(transformLoc, transforms.size(), GL_FALSE, glm::value_ptr(transforms[0]));
    }

    state.num_points = points.size();
    std::cout << "Starting generation with " << state.num_points << " points.\n";

    int ITERATION_DEPTH = 2; // Number of iterations performing point generation
    variableses.num_generations = ITERATION_DEPTH;
    int read_idx = generate_points(variableses.num_generations, VAOs, tfos, generateProgram, state.num_points);
    state.num_points *= glm::pow(4, variableses.num_generations);

    std::cout << state.num_points << std::endl;
    // ------------------ Final Drawing Step ------------------------


    // Unbind to keep a clean state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glPointSize(1.0f);

    glUseProgram(drawProgram); // Use the program for drawing

    bool g_key_was_pressed = false;
    int press_counter = 0;
    while (!glfwWindowShouldClose(window))
    {
        // Calculate the timings
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(58.0f/255.0f, 61.0f/255.0f, 59.0f/255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(drawProgram);

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float aspectRatio = (float)width / (float)height;

        glm::mat4 projection = glm::ortho(-orthoSize * aspectRatio, orthoSize * aspectRatio, -orthoSize, orthoSize, -1.0f, 1.0f);

        // View: creates a matrix that "looks" from cameraPos to a target
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // 4. Upload Matrices to the Shader
        int projLoc = glGetUniformLocation(drawProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        int viewLoc = glGetUniformLocation(drawProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Input handling
        processInput(window, variableses);
        processCameraInput(window, deltaTime, variableses, state);
        // Regenerate the points if needed
        bool g_key_is_down = (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS);
        if (g_key_is_down && !g_key_was_pressed)
        {
            // This block will now only execute ONCE per key press
            variableses.num_generations++;
            read_idx = generate_points(variableses.num_generations, VAOs, tfos, generateProgram, state.num_points);
            state.num_points *= glm::pow(4, variableses.num_generations);
            std::cout << "Generated points for generation: " << variableses.num_generations << std::endl;
        }

        // if (g_key_is_down && !g_key_was_pressed) {
        //     press_counter++;
        //     std::cout << "Key 'G' pressed. Count: " << press_counter << std::endl;
        // }

        g_key_was_pressed = g_key_is_down;

        // Rendering
        glUseProgram(drawProgram);

        glBindVertexArray(VAOs[read_idx]);
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
