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


#include "input.h"
#include "shaders.h"
#include "renderer.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  

std::vector<glm::vec4> transformPoints(std::vector<glm::vec4> startPoints, std::vector<glm::mat4> transforms){
    std::vector<glm::vec4> resault;
    for(const auto& transform: transforms){
        for(const auto& point: startPoints){
            resault.push_back(transform * point);
        }
    }

    // TODO: Remove duplicates?

    return resault;
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

    Renderer renderer;

    // Points and matrix data:
    std::vector<glm::vec4> points = {
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
    };

    // float trans1[] = {
    //     -0.67f, -0.02f, 0.0f, 0.0f,
    //     -0.18f, 0.81f, 0.0f, 10.0f,
    //     0.0f, 0.0f, 1.0f, 0.0f,
    //     0.0f, 0.0f, 0.0f, 1.0f
    // };


    // float trans2[] = {
    //     0.4f, 0.4f, 0.0f, 0.0f,
    //     -0.1f, 0.4f, 0.0f, 0.0f,
    //     0.0f, 0.0f, 1.0f, 0.0f,
    //     0.0f, 0.0f, 0.0f, 1.0f
    // };

    // float trans3[] = {
    //     -0.4f, -0.4f, 0.0f, 0.0f,
    //     -0.1f, 0.4f, 0.0f, 0.0f,
    //     0.0f, 0.0f, 1.0f, 0.0f,
    //     0.0f, 0.0f, 0.0f, 1.0f
    // };

    // float trans4[] = {
    //     -0.1f, 0.0f, 0.0f, 0.0f,
    //     0.44f, 0.44f, 0.0f, -2.0f,
    //     0.0f, 0.0f, 1.0f, 0.0f,
    //     0.0f, 0.0f, 0.0f, 1.0f
    // };


    glm::mat4 baseScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 1.0f));

    // Transformation 1: Scale by 0.5, then move to the bottom-left corner (no translation).
    glm::mat4 transform1 = glm::translate(baseScale, glm::vec3(0.0f, 0.0f, 0.0f));

    // Transformation 2: Scale by 0.5, then move to the bottom-right corner.
    // The shrunken triangle's corner is at (0,0), we move it to (0.5, 0).
    glm::mat4 transform2 = glm::translate(baseScale, glm::vec3(0.5f, 0.0f, 0.0f));

    // Transformation 3: Scale by 0.5, then move to the top corner.
    // The shrunken triangle's corner is at (0,0), we move it to (0.25, 0.5).
    glm::mat4 transform3 = glm::translate(baseScale, glm::vec3(0.25f, 0.5f, 0.0f));

    // Your shader expects an array of 4 matrices. We'll just make the fourth one
    // the identity matrix so it does nothing.
    glm::mat4 transform4 = glm::mat4(1.0f);

    std::vector<glm::mat4> transforms = {transform1, transform2, transform3 ,transform4};

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

    GLuint query;
    glGenQueries(1, &query);

    // Disable rasterizer for the generation program
    glEnable(GL_RASTERIZER_DISCARD);

    int points_in_buffer = points.size();
    int read_idx = 0; // Index for the read index

    std::cout << "Starting generation with " << points_in_buffer << " points.\n";


    int ITERATION_DEPTH = 11; // Number of iterations performing point generation
    // Accual generation calls:
    for (int i = 0; i < ITERATION_DEPTH; i++)
    {
        int write_idx = 1 - read_idx; // Index for the write index

        glBindVertexArray(VAOs[read_idx]);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfos[write_idx]);

         if (i == ITERATION_DEPTH - 1) {
            glBeginQuery(GL_PRIMITIVES_GENERATED, query);
        }

        // Perform the transformation feedback
        glBeginTransformFeedback(GL_POINTS);
        glDrawArrays(GL_POINTS, 0, points_in_buffer);
        glEndTransformFeedback();

        if (i == ITERATION_DEPTH - 1) {
            glEndQuery(GL_PRIMITIVES_GENERATED);
        }

        points_in_buffer *= 4; // Number of points in buffer after 4 transformations
        // Swap read index for next iteration
        read_idx = 1 - read_idx;
    }
    
    // --- FIX #3: Get the query result AFTER the loop ---
    GLuint primitives_written = 0;
    // This call will wait until the GPU is done and give you the real number.
    glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives_written);
    std::cout << "GPU reported generating " << primitives_written << " points in the final iteration.\n";

    // Reenable rasterizer
    glDisable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0); // Unbind the transform

    // ------------------ Final Drawing Step ------------------------


    // Unbind to keep a clean state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glPointSize(1.0f);

    glUseProgram(drawProgram); // Use the program for drawing


    while (!glfwWindowShouldClose(window))
    {
        // Calculate the timings
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
    
        // Input handling
        processInput(window);
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

        // Rendering

        glBindVertexArray(VAOs[read_idx]);
        glDrawArrays(GL_POINTS, 0, points_in_buffer);
        
        // Debug printing
        std::cout << 1 - orthoSize << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
