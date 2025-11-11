#version 440 core

layout(location = 0) in vec4 aPos;

void main() {
    // Just pass the position through to the Geometry Shader
    gl_Position = aPos;
}