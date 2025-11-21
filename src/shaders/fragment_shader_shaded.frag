#version 440 core

in float v_ndc_z; // Input from the vertex shader

out vec4 FragColor;

uniform vec3 baseColor = vec3(32.0/255.0, 171.0/255.0, 69.0/255.0);
uniform vec3 fogColor = vec3(0.227, 0.239, 0.231); // 58, 61, 59
uniform float fogDensity = 1.2;

// Map NDC z in [-1, +1] to [0, 1] (0 = near, 1 = far)
float ndcTo01(float zndc) {
    return (zndc + 1.0) * 0.5;
}

void main() {
    float z01 = ndcTo01(v_ndc_z);

    // Exponential fog factor (0 near, ->1 far)
    float fogFactor = 1.0 - exp(-fogDensity * fogDensity * z01 * z01);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 shaded = mix(baseColor, fogColor, fogFactor);

    FragColor = vec4(shaded, 1.0);
}