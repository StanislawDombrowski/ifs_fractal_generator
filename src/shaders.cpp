#include "shaders.h"

std::string readShader(const char* filePath){
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);
    if(!fileStream.is_open()){
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }

    std::string line;
    while (std::getline(fileStream, line)){
        content += line + "\n";
    }
    fileStream.close();
    return content;
}

unsigned int compileShader(const std::string& shaderSourceCStr, unsigned int shaderType){
    unsigned int shader = glCreateShader(shaderType);
    const char* shaderSource = shaderSourceCStr.c_str();
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    // Check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        if(shaderType == GL_VERTEX_SHADER){
            std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        } else if(shaderType == GL_FRAGMENT_SHADER){
            std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        } else {
            std::cerr << "ERROR::SHADER::UNKNOWN_TYPE::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }
    return shader;
}

unsigned int createShaderProgram(std::vector<unsigned int> shaders){
    unsigned int shaderProgram = glCreateProgram();
    for(unsigned int shader: shaders){
        glAttachShader(shaderProgram, shader);
    }
    glLinkProgram(shaderProgram);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    return shaderProgram;
}