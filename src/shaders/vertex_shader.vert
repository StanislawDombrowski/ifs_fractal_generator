#version 440 core

layout(location = 0) in dvec4 aPos;

// Create a custom output variable to carry the high-precision data
out dvec4 high_precision_pos;

void main() {
    // Pass the double-precision data through our custom variable
    high_precision_pos = aPos;
    
    // Write a dummy (but valid) value to the mandatory gl_Position.
    // The GS will not use this.
    gl_Position = vec4(aPos); 
}