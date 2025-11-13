#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

std::string readShader(const char* filePath);

unsigned int compileShader(const std::string& shaderSourceCStr, unsigned int shaderType);
unsigned int createShaderProgram(std::vector<unsigned int> shaders);