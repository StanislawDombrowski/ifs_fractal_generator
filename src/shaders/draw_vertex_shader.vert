#version 440 core

layout(location = 0) in dvec4 aPos;

uniform dmat4 projection;
uniform dmat4 view;

void main() {
    // Just pass the position through to the Geometry Shader
    gl_Position = vec4(projection * view * aPos);
}