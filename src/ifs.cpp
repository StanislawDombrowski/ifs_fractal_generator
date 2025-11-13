#include "ifs.h"

int generate_points(int depth, ifs_state &state, unsigned int* tfos, unsigned int program, int start_read_idx){
    GLuint query;
    glGenQueries(1, &query);

    // Disable rasterizer for the generation program
    glUseProgram(program);
    glEnable(GL_RASTERIZER_DISCARD);
    
    int points_in_buffer = state.num_points;
    int read_idx = start_read_idx;

    for (int i = 0; i < depth; i++)
    {
        int write_idx = 1 - read_idx; // Index for the write index

        glBindVertexArray(state.VAOs[read_idx]);

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

        points_in_buffer *= state.num_of_transforms;
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
    state.num_points = points_in_buffer;

    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
    return read_idx;
}

std::vector<glm::dmat4> init_transforms(std::string matrix_source, ifs_state &state){
    std::ifstream file(matrix_source);

    std::vector<double> indicies;
    double num;
    char space;

     while (file >> num) {
        indicies.push_back(num);
        file.get(space); // Consume the space character
    }

    file.close();

    state.num_of_transforms = indicies[0];
    std::vector<glm::dmat4> matrices;

    for (int j = 0; j < state.num_of_transforms; j++)
    {
        glm::dmat4 matrix = glm::dmat4(
            glm::dvec4(indicies[1 + (16 * j)], indicies[1 + 1 + (16 * j)], indicies[1 + 2 + (16 * j)], indicies[1 + 3 + (16 * j)]),
            glm::dvec4(indicies[1 + 4 + (16 * j)], indicies[1 + 5 + (16 * j)], indicies[1 + 6 + (16 * j)], indicies[1 + 7 + (16 * j)]),
            glm::dvec4(indicies[1 + 8 + (16 * j)], indicies[1 + 9 + (16 * j)], indicies[1 + 10 + (16 * j)], indicies[1 + 11 + (16 * j)]),
            glm::dvec4(indicies[1 + 12 + (16 * j)], indicies[1 + 13 + (16 * j)], indicies[1 + 14 + (16 * j)], indicies[1 + 15 + (16 * j)])
            );
            matrices.push_back(matrix);   
    }

    // for (int i = 0; i < 4 - size; i++)
    // {
    //     glm::dmat4 matrix = glm::dmat4(1.0);
    //     matrices.push_back(matrix);
    // }

    return matrices;
}

void update_IFS_data(ifs_state &state, std::vector<glm::dmat4> transforms, unsigned int program){
    
    int transformLoc = glGetUniformLocation(program, "transformations");
    int num_of_tansformsLoc = glGetUniformLocation(program, "num_of_transforms");

    glUniformMatrix4dv(transformLoc, transforms.size(), GL_FALSE, glm::value_ptr(transforms[0]));
    glUniform1i(num_of_tansformsLoc, state.num_of_transforms);
}