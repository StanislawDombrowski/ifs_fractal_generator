#include "ifs.h"

int generate_points(int depth, unsigned int* VAOs, unsigned int* tfos, unsigned int program, int num_points){
    GLuint query;
    glGenQueries(1, &query);

    std::cout << depth << std::endl;
    // Disable rasterizer for the generation program
    glEnable(GL_RASTERIZER_DISCARD);
    glUseProgram(program);

    int points_in_buffer = num_points;
    int read_idx = 0; // Index for the read index

    for (int i = 0; i < depth; i++)
    {
        int write_idx = 1 - read_idx; // Index for the write index

        glBindVertexArray(VAOs[read_idx]);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfos[write_idx]);

         if (i == depth - 1) {
            glBeginQuery(GL_PRIMITIVES_GENERATED, query);
        }

        // Perform the transformation feedback
        glBeginTransformFeedback(GL_POINTS);
        glDrawArrays(GL_POINTS, 0, points_in_buffer);
        glEndTransformFeedback();

        if (i == depth - 1) {
            glEndQuery(GL_PRIMITIVES_GENERATED);
        }

        points_in_buffer *= 4; // Number of points in buffer after 4 transformations
        // Swap read index for next iteration
        read_idx = 1 - read_idx;
    }

    // --- FIX #3: Get the query result AFTER the loop ---
    GLuint primitives_written = 0;
    // This call will wait until the GPU is done and give you the real number.
    glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives_written);
    std::cout << "GPU reported generating " << primitives_written << " points in the final iteration.\n";

    // Reenable rasterizer
    glDisable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0); // Unbind the transform
    num_points = points_in_buffer;
    return read_idx;
}

std::vector<glm::mat4> init_transforms(std::string matrix_source){
    std::ifstream file(matrix_source);

    std::vector<float> indicies;
    float num;
    char space;

     while (file >> num) {
        indicies.push_back(num);
        file.get(space); // Consume the space character
    }

    file.close();

    int size = indicies[0];
    std::vector<glm::mat4> matrices;

    for (int i = 1; i < indicies.size(); i++)
    {  
        
    }

    for (int j = 0; j < size; j++)
    {
        glm::mat4 matrix = glm::mat4(
            glm::vec4(indicies[1 + (16 * j)], indicies[1 + 1 + (16 * j)], indicies[1 + 2 + (16 * j)], indicies[1 + 3 + (16 * j)]),
            glm::vec4(indicies[1 + 4 + (16 * j)], indicies[1 + 5 + (16 * j)], indicies[1 + 6 + (16 * j)], indicies[1 + 7 + (16 * j)]),
            glm::vec4(indicies[1 + 8 + (16 * j)], indicies[1 + 9 + (16 * j)], indicies[1 + 10 + (16 * j)], indicies[1 + 11 + (16 * j)]),
            glm::vec4(indicies[1 + 12 + (16 * j)], indicies[1 + 13 + (16 * j)], indicies[1 + 14 + (16 * j)], indicies[1 + 15 + (16 * j)])
            );
            matrices.push_back(matrix);   
    }

    for (int i = 0; i < 4 - size; i++)
    {
        glm::mat4 matrix = glm::mat4(1.0f);
        matrices.push_back(matrix);
    }

    return matrices;
}