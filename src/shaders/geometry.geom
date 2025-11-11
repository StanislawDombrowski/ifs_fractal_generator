#version 440 core
layout (points) in;
// Output is still points, but max_vertices is now small!
// We only generate 4 points for every 1 input point.
layout (points, max_vertices = 4) out;

uniform mat4 transformations[4];

// We need to tell the shader which output variable to "capture"
out vec4 out_Pos;

void main() {
    // For each input point, generate the next 4
    for(int j = 0; j < 4; j++) {
        out_Pos = transformations[j] * gl_in[0].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}