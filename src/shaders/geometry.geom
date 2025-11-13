#version 440 core
layout (points) in;
// Output is still points, but max_vertices is now small!
// We only generate 4 points for every 1 input point.
layout (points, max_vertices = 4) out;

uniform dmat4 transformations[4];
uniform int num_of_transforms;

in dvec4 high_precision_pos[];

// We need to tell the shader which output variable to "capture"
out dvec4 out_Pos;

void main() {
    // For each input point, generate the next 4
    for(int j = 0; j < num_of_transforms; j++) {
        out_Pos = transformations[j] * high_precision_pos[0];
        EmitVertex();
    }
    EndPrimitive();
}