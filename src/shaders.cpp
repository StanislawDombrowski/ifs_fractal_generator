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

unsigned int compileShader(const std::string& source, GLenum type) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    // --- ERROR CHECKING ---
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        
        std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment/geometry") << " shader!" << std::endl;
        std::cerr << message << std::endl;
        
        glDeleteShader(id);
        return 0;
    }

    return id;
}

// A more robust function to create a shader program
unsigned int createShaderProgram(const std::vector<unsigned int>& shaders) {
    unsigned int program = glCreateProgram();
    for (unsigned int shader : shaders) {
        glAttachShader(program, shader);
    }
    glLinkProgram(program);

    // --- ERROR CHECKING ---
    int result;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetProgramInfoLog(program, length, &length, message);

        std::cerr << "Failed to link shader program!" << std::endl;
        std::cerr << message << std::endl;

        glDeleteProgram(program);
        // You might also want to delete the individual shaders here
        return 0;
    }

    for(unsigned int shader: shaders){
        glDeleteShader(shader);
    }

    // You can also validate the program, though it's often redundant with linking
    // glValidateProgram(program);

    return program;
}