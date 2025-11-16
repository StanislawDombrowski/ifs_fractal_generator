#version 440 core

layout(location = 0) in dvec4 in_Pos;

uniform dmat4 projection;
uniform dmat4 view;

// This is the variable we pass to the fragment shader
out float v_ndc_z;

void main() {
    // Perform high-precision calculations
    dvec4 clip_pos = projection * view * in_Pos;

    // Perspective divide to get Normalized Device Coordinates (NDC)
    // The result is in [-1, 1] range.
    if (clip_pos.w > 0.0) {
        v_ndc_z = float(clip_pos.z / clip_pos.w);
    } else {
        v_ndc_z = 1.0; // Place points behind the camera at the far plane
    }
    
    // **THE FIX:** Explicitly cast the double-precision dvec4
    // to a single-precision vec4 for gl_Position.
    gl_Position = vec4(clip_pos);
}