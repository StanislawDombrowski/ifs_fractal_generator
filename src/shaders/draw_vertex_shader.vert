#version 440 core

// 1. INPUT: Must match glVertexAttribLPointer (dvec4 for GL_DOUBLE)
layout (location = 0) in dvec4 aPos; 

// 2. UNIFORMS: Must match glUniformMatrix4dv (dmat4 for doubles)
uniform dmat4 projection;
uniform dmat4 view;

void main()
{
    // 3. CALCULATION: Perform math in double precision
    dvec4 pos = projection * view * aPos;

    // 4. OUTPUT: Convert to float (vec4) for the rasterizer (screen only supports float)
    gl_Position = vec4(pos);
}