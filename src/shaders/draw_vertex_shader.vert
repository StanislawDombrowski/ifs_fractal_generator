#version 440 core

layout(location = 0) in vec4 aPos;

uniform mat4 projection;
uniform mat4 view;

void main() {
    // Just pass the position through to the Geometry Shader
    gl_Position = projection * view * aPos;
}