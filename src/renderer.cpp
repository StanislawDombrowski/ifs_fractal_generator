#include "renderer.h"

Renderer::Renderer(){

}

Renderer::~Renderer(){
    
}
unsigned int Renderer::initShaders(std::vector<std::string> sourcesPath, std::vector<GLenum> types){
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

std::array<unsigned int, 2> Renderer::initVBOs() {
    std::array<unsigned int, 2> VBOs;
    glGenBuffers(2, VBOs.data());
    return VBOs;
}

std::array<unsigned int, 2> Renderer::initVAOs(const int MAX_POINTS, std::array<unsigned int, 2> VBOs) {
    std::array<unsigned int, 2> VAOs;
    glGenVertexArrays(2, VAOs.data());
    for (size_t i = 0; i < 2; i++)
    {
        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec4) * MAX_POINTS, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribLPointer(0, 4, GL_DOUBLE, 0, (void*)0);
    }
    return VAOs;
}

std::array<unsigned int, 2> Renderer::initTFOs(std::array<unsigned int, 2> VBOs){
    std::array<unsigned int, 2> tfos;
    glGenTransformFeedbacks(2, tfos.data());


    // Set up transform feedback to capture data into the VBOs
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfos[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBOs[0]);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfos[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, VBOs[1]);

    // Unbind the tfos
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    return tfos;
}

void Renderer::fillVBO(unsigned int VBO, std::vector<glm::dvec4> points){
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(glm::dvec4), points.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

float Renderer::calculateDeltaTime(double deltaTime)
{
    double now = glfwGetTime();
    deltaTime = now - lastFrame;
    lastFrame = now;

    return deltaTime;
}

void Renderer::sendData(Input &input)
{
    glUseProgram(shader);
    glUniformMatrix4dv(glGetUniformLocation(shader, "projection"),
                            1, GL_FALSE, glm::value_ptr(input.camera.projection));
    glUniformMatrix4dv(glGetUniformLocation(shader, "view"),
                            1, GL_FALSE, glm::value_ptr(input.camera.view));
}

int Renderer::calculateDrawIndex(Input &input, const IFS& ifs)
{
    double target_points = (1.0 / (input.camera.orthoSize * input.camera.orthoSize)) * 5000000.0 * detailFactor;
    int draw_index = 0;
    for(int i = 0; i < ifs.state.history.size(); i++) {
        if(ifs.state.history[i].point_count > target_points) {
            draw_index = i;
            break; // Found a generation with enough detail
        }
        draw_index = i; // Default to the highest available if we never exceed target
    }

    return draw_index;
}

void Renderer::render(GLFWwindow* window, Input &input, IFS& ifs)
{
    deltaTime = calculateDeltaTime(deltaTime);
    sendData(input);
    int draw_index = calculateDrawIndex(input, ifs);
    ifs.state.draw_index = draw_index;

    glDisable(GL_BLEND); 
    glDisable(GL_DEPTH_TEST);

    if (!ifs.state.history.empty()) { // Safety check
        glBindVertexArray(ifs.state.history[draw_index].vao);
        std::cout << ifs.state.history[draw_index].point_count << std::endl;
        glDrawArrays(GL_POINTS, 0, ifs.state.history[draw_index].point_count);
    }
    
}

