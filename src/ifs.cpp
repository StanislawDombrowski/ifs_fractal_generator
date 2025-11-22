#include "ifs.h"
#include <stdexcept>

IFS::IFS(){

}

IFS::~IFS(){
    
}

// --- Helper: Allocates buffers for a new generation ---
GenerationData allocate_new_generation(int count) {
    GenerationData gen;
    gen.point_count = count;

    // 1. Generate GL objects
    glGenVertexArrays(1, &gen.vao);
    glGenBuffers(1, &gen.vbo);
    glGenTransformFeedbacks(1, &gen.tfo);

    // 2. Setup VBO (Allocate memory, but don't fill yet)
    glBindBuffer(GL_ARRAY_BUFFER, gen.vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::dvec4), nullptr, GL_STATIC_DRAW);

    // 3. Setup VAO (How to read this VBO)
    glBindVertexArray(gen.vao);
    glEnableVertexAttribArray(0);
    // Important: glVertexAttribLPointer for GL_DOUBLE
    glVertexAttribLPointer(0, 4, GL_DOUBLE, 0, (void*)0);

    // 4. Setup TFO (How to write into this VBO)
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, gen.tfo);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, gen.vbo);

    // Cleanup binding
    glBindVertexArray(0);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return gen;
}

unsigned int IFS::initShaders(std::vector<std::string> sourcesPath, std::vector<GLenum> types){
    unsigned int shader;

    std::vector<unsigned int> individuals;
    individuals.reserve(sourcesPath.size());

    std::vector<std::string> sources;
    sources.reserve(sourcesPath.size());
    for(std::string &source: sourcesPath){
        sources.push_back(readShader(source.c_str()));
    }

    std::map<std::string, GLenum> pairs;

    for (int i = 0; i < sourcesPath.size(); i++)
    {
        pairs.insert({sources.at(i), types.at(i)});
    }
    

    for(std::string &source: sources){
        individuals.push_back(compileShader(source, pairs.at(source)));
    }

    shader = createShaderProgram(individuals);
    return shader;
}

void IFS::generate_points(int depth, ifs_state &state, unsigned int program) {
    if (state.history.empty()) return;

    GLuint query;
    glGenQueries(1, &query);

    glUseProgram(program);
    glEnable(GL_RASTERIZER_DISCARD);

    for (int i = 0; i < depth; i++) {
        // 1. Identify Source (The last generation we calculated)
        int source_idx = state.history.size() - 1;
        int points_to_read = state.history[source_idx].point_count;

        // 2. Prepare Destination (Step B: Allocate New Buffers)
        int expected_points = points_to_read * state.num_of_transforms;
        GenerationData next_gen = allocate_new_generation(expected_points);

        // 3. Bind Source for Reading
        glBindVertexArray(state.history[source_idx].vao);

        // 4. Bind Destination for Writing (Transform Feedback)
        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, next_gen.tfo);

        // 5. Execute Shader
        glBeginQuery(GL_PRIMITIVES_GENERATED, query);
        glBeginTransformFeedback(GL_POINTS);
        
        glDrawArrays(GL_POINTS, 0, points_to_read);
        
        glEndTransformFeedback();
        glEndQuery(GL_PRIMITIVES_GENERATED);

        state.history.at(state.draw_index).point_count *= state.num_of_transforms;

        // 6. Verify Output
        GLuint primitives_written = 0;
        glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives_written);
        
        // Update the actual count (in case shader discarded some, though unlikely here)
        next_gen.point_count = primitives_written;
        
        // 7. Add to History
        state.history.push_back(next_gen);
    }

    glDisable(GL_RASTERIZER_DISCARD);
    glDeleteQueries(1, &query);
}

void IFS::init_genesis(ifs_state &state, std::vector<glm::dvec4>& initial_points) {
    state.history.clear();
    GenerationData seed_gen = allocate_new_generation(initial_points.size());
    
    // Manually upload the CPU data to the first VBO
    glBindBuffer(GL_ARRAY_BUFFER, seed_gen.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, initial_points.size() * sizeof(glm::dvec4), initial_points.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    state.history.push_back(seed_gen);
}

std::vector<glm::dmat4> IFS::init_transforms(std::string matrix_source, ifs_state &state){
    std::ifstream file(matrix_source);

    std::vector<double> indicies;
    double num;
    char space;

     while (file >> num) {
        indicies.push_back(num);
        file.get(space); // Consume the space character
    }

    file.close();

    if (indicies.empty()) {
        throw std::runtime_error("No data read from: " + std::string(matrix_source));
    }   

    state.num_of_transforms = indicies.at(0);
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

    return matrices;
}

void IFS::update_IFS_data(ifs_state &state, std::vector<glm::dmat4> transforms, unsigned int program){
    
    int transformLoc = glGetUniformLocation(program, "transformations");
    int num_of_tansformsLoc = glGetUniformLocation(program, "num_of_transforms");

    glUniformMatrix4dv(transformLoc, transforms.size(), GL_FALSE, glm::value_ptr(transforms[0]));
    glUniform1i(num_of_tansformsLoc, state.num_of_transforms);
}