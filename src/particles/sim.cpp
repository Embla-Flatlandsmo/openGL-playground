#include "sim.hpp"
#include "boid.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
/* Local utilities */
#include <utilities/shader.hpp>
#include <utilities/camera.hpp>


/* Particle information */
glm::vec3 *positions;

/* Shaders */
Gloom::Shader *colorShader;
// Gloom::Shader *computeShader;

void initShaders()
{
    colorShader = new Gloom::Shader();
    colorShader->makeBasicShader("res/shaders/particle.vert", "res/shaders/particle.frag");
    
    // computeShader = new Gloom::Shader();
    // computeShader->attach("res/shaders/particle.comp");
}

#define BASE 4 // number of primitives per boid
#define VERTCOUNT (BASE + 1) * 3
#define IDXCOUNT BASE * 3
#define BOXVERTSIZE 8 * 3
#define BOXELEMSIZE 24

void initBuffers()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /* Boid geometry setup */
    float verts[VERTCOUNT];
    for (size_t i = 0; i < BASE; i++)
    {
        verts[i * 3] = 0;
        verts[i * 3 + 1] = sin(i * 2 * M_PI / (BASE)) * boidSize;
        verts[i * 3 + 2] = cos(i * 2 * M_PI / (BASE)) * boidSize;
    }
    verts[BASE * 3] = 2 * boidSize,
                verts[BASE * 3 + 1] = 0,
                verts[BASE * 3 + 2] = 0;

    unsigned int indices[IDXCOUNT];
    for (size_t i = 0; i < BASE; i++)
    {
        indices[i * 3] = i,
                    indices[i * 3 + 1] = (i + 1) % BASE,
                    indices[i * 3 + 2] = BASE;
    }

    //vertices
    glGenBuffers(1, &bufGeometry);
    glBindBuffer(GL_ARRAY_BUFFER, bufGeometry);
    glBufferData(GL_ARRAY_BUFFER, VERTCOUNT * sizeof(float), verts, GL_STATIC_DRAW);

    colorShader->activate();
    GLint vertLoc = glGetAttribLocation(colorShader->get(), "vert");
    glVertexAttribPointer(vertLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertLoc);

    // Indices
    glGenBuffers(1, &ebufGeometry);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebufGeometry);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IDXCOUNT * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // //AGENT BUFFERS SETUP
    // glGenBuffers(1, &bufAgents);
    // glBindBuffer(GL_ARRAY_BUFFER, bufAgents);
    // glBufferData(GL_ARRAY_BUFFER, agents->size * sizeof(Boid), nullptr, GL_DYNAMIC_COPY);

    // GLint posLoc = glGetAttribLocation(colorShader->get(), "position");
    // GLint velLoc = glGetAttribLocation(colorShader->get(), "velocity");
    // GLint colLoc = glGetAttribLocation(colorShader->get(), "colorValue");


    // glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Boid), 0);
    // glEnableVertexAttribArray(posLoc);
    // glVertexAttribDivisor(posLoc, 1);

    // glVertexAttribPointer(velLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Boid), (void *)(3 * sizeof(float)));
    // glEnableVertexAttribArray(velLoc);
    // glVertexAttribDivisor(velLoc, 1);

    // glVertexAttribPointer(colLoc, 1, GL_FLOAT, GL_FALSE, sizeof(Boid), (void *)(9 * sizeof(float) + sizeof(uint32_t)));
    // glEnableVertexAttribArray(colLoc);
    // glVertexAttribDivisor(colLoc, 1);

    // glVertexAttribPointer(agentProgram.attr["position"], 3, GL_FLOAT, GL_FALSE, sizeof(Boid), 0);
    // glEnableVertexAttribArray(agentProgram.attr["position"]);
    // glVertexAttribDivisor(agentProgram.attr["position"], 1);

    // glVertexAttribPointer(agentProgram.attr["velocity"], 3, GL_FLOAT, GL_FALSE, sizeof(Boid), (void *)(3 * sizeof(float)));
    // glEnableVertexAttribArray(agentProgram.attr["velocity"]);
    // glVertexAttribDivisor(agentProgram.attr["velocity"], 1);

    // glVertexAttribPointer(agentProgram.attr["colorValue"], 1, GL_FLOAT, GL_FALSE, sizeof(Boid), (void *)(9 * sizeof(float) + sizeof(uint32_t)));
    // glEnableVertexAttribArray(agentProgram.attr["colorValue"]);
    // glVertexAttribDivisor(agentProgram.attr["colorValue"], 1);

    //setup SSBO for updating
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufAgents);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufAgents); // buffer on binding 0


}

void drawBoids(GLFWwindow *window, Gloom::Camera *camera) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), float(windowWidth) / float(windowHeight), 0.5f, 10000.f);

    glBindVertexArray(vao);
    colorShader->activate();
    glUniformMatrix4fv(colorShader->getUniformFromName("view"), 1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));
    glUniformMatrix4fv(colorShader->getUniformFromName("projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glDrawElements(GL_TRIANGLES, IDXCOUNT, GL_UNSIGNED_INT, nullptr);
    // glDrawElementsInstanced(GL_TRIANGLES, IDXCOUNT, GL_UNSIGNED_INT, (void *)0, agents->size);
}

// void setupBoids()
// {
//     glBindVertexArray(vao);
//     glBindBuffer(GL_ARRAY_BUFFER, bufAgents);
//     glBufferSubData(GL_ARRAY_BUFFER, 0, agents->size * sizeof(Boid), agents->boids);
// }

// void computeShaderNaiveFlock()
// {
//     //bind uniforms
//     computeShader->activate();
//     glUniform1ui(naiveFlockProgram.unif["size"], agents->size);

//     glDispatchCompute(groups, 1, 1);
//     glMemoryBarrier(GL_ALL_BARRIER_BITS);

//     getGLError("gpu naive flocking");
// }