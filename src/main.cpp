#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>

#include "input.h"
#include "shaders.h"
#include "renderer.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  


int main(){

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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
    glEnable(GL_PROGRAM_POINT_SIZE);

    pointRenderer point_renderer;

    // Initialize vertex data
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };

    std::vector<glm::vec3> points = {
        glm::vec3(-0.5f, -0.5f, 0.0f),
        glm::vec3(0.5f, -0.5f, 0.0f),
        glm::vec3(0.0f, 0.5f, 0.0f)
    };

    float vertex[] = {0.0f, 0.0f, 0.0f};

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 

    // Load and compile shaders
    const std::string vertexShaderSource = readShader("C:\\dev\\openGL_learning\\src\\vertex_shader.vert");
    const std::string fragmentShaderSource = readShader("C:\\dev\\openGL_learning\\src\\fragment_shader.frag");


    // Compile vertex shader
    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    // Compile fragment shader
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    // Link shaders to create shader program
    unsigned int shaderProgram = createShaderProgram(vertexShader, fragmentShader);

    // Individual shader cleanup
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int VBO = point_renderer.initVBO();
    unsigned int VAO = point_renderer.initVAO();

    point_renderer.setPointSize(10.0f);
    point_renderer.renderPoints(VBO, points);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0); 

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, 3);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
