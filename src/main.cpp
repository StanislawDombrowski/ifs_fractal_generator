#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <iostream>
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

void processCameraInput(GLFWwindow *window, float dt)
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


    input_variables variables;
    variables.should_generate = false;
    Renderer renderer;

    // Points and matrix data:
    std::vector<glm::vec4> points = {
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
    };

    std::vector<glm::mat4> transforms = init_transforms("C:\\dev\\ifs-fractals\\transformations\\leaf.txt");

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 

    // Load and compile shaders for generation
    const std::string vertexShaderSource = readShader("C:\\dev\\ifs-fractals\\src\\shaders\\vertex_shader.vert");
    const std::string geometryShaderSource = readShader("C:\\dev\\ifs-fractals\\src\\shaders\\geometry.geom");
    const std::string fragmentShaderSource = readShader("C:\\dev\\ifs-fractals\\src\\shaders\\fragment_shader.frag");

    // Shaders for drawing
    const std::string drawVertexShaderSource = readShader("C:\\dev\\ifs-fractals\\src\\shaders\\draw_vertex_shader.vert");

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

    int points_in_buffer = points.size();
    std::cout << "Starting generation with " << points_in_buffer << " points.\n";

    int ITERATION_DEPTH = 1; // Number of iterations performing point generation
    variables.num_generations = ITERATION_DEPTH;
    int read_idx = generate_points(variables.num_generations, VAOs, tfos, generateProgram, points_in_buffer);

    std::cout << points_in_buffer << std::endl;
    // ------------------ Final Drawing Step ------------------------


    // Unbind to keep a clean state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glPointSize(1.0f);

    glUseProgram(drawProgram); // Use the program for drawing

    std::cout << variables.should_generate << std::endl;
    while (!glfwWindowShouldClose(window))
    {
        // Calculate the timings
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
    
        // Input handling
        processInput(window, variables);
        processCameraInput(window, deltaTime);

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

        // Regenerate the points if needed
        if(variables.should_generate){
            read_idx = generate_points(variables.num_generations, VAOs, tfos, generateProgram, points_in_buffer);
            variables.should_generate = false;
        }
        std::cout << variables.should_generate << std::endl;
        // Rendering
        glUseProgram(drawProgram);

        glBindVertexArray(VAOs[read_idx]);
        glDrawArrays(GL_POINTS, 0, points_in_buffer);
        
        // Debug printing
        //std::cout << 1 - orthoSize << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
