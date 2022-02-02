// Local headers
#include "particleSystem.hpp"
#include <utilities/shader.hpp>
#include <particles/boundingBox.hpp>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <utilities/objLoader.hpp>

// System headers
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#define WORK_GROUP_SIZE 128

#define MAX_VELOCITY 2

ParticleSystem::ParticleSystem(glm::vec3 low, glm::vec3 high)
{
    particleModel = new Mesh(loadObj("./res/models/boid.obj"));
    particleSize = 0.5;
    boundingBox = new BoundingBox(low, high);

    particlePoints = new struct pos[NUM_PARTICLES];
    particleVels = new struct vel[NUM_PARTICLES];
    particleCols = new struct color[NUM_PARTICLES];

    colorShader = new Gloom::Shader();
    colorShader->makeBasicShader("res/shaders/particle.vert", "res/shaders/particle.frag");
    colorShader->activate();
    glUniform1f(colorShader->getUniformFromName("particleSize"), particleSize);
    colorShader->deactivate();
    
    // The compute shader must be in its own program
    computeShader = new Gloom::Shader();
    computeShader->attach("res/shaders/particle.comp");
    computeShader->link();

    // We set the uniforms for the bounding box in the compute shader only once
    computeShader->activate();
    glUniform3fv(computeShader->getUniformFromName("boundingBoxLow"), 1, glm::value_ptr(boundingBox->low));
    glUniform3fv(computeShader->getUniformFromName("boundingBoxHigh"), 1, glm::value_ptr(boundingBox->high));
    computeShader->deactivate();
    initParticles();
}

ParticleSystem::~ParticleSystem()
{
    // Some cleanup here is probably needed...
    computeShader->destroy();
    colorShader->destroy();
}

void ParticleSystem::update()
{
    computeShader->activate();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, particlePosSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particleVelSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, particleColSSBO);

    glDispatchCompute(NUM_PARTICLES/WORK_GROUP_SIZE, 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    computeShader->deactivate();
}

/**
 * @brief Render the particles
 *
 * @param window The window that openGL runs in
 * @param camera Camera position
 * @param debug true to show the bounding box
 */
void ParticleSystem::render(GLFWwindow *window, Gloom::Camera *camera)
{
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    // We set a projection matrix to get some perspective going!
    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
    glm::mat4 VP = projection * camera->getViewMatrix();

    if (debug)
    {
        boundingBox->renderAsWireframe(window, camera);
    }
    colorShader->activate();
    glUniformMatrix4fv(colorShader->getUniformFromName("VP"), 1, GL_FALSE, glm::value_ptr(VP));

    glBindVertexArray(particleVAO);
    // Finally we draw the particles
    // glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
    glDrawElementsInstanced(GL_TRIANGLES, particleModel->indices.size(), GL_UNSIGNED_INT, (void *)0, NUM_PARTICLES);
    colorShader->deactivate();
}

void ParticleSystem::setDebugMode(bool debug)
{
    this->debug = debug;
}

/**
 * @brief Helper function for initializing the particles.
 * This is where VAO is created and the SSBOs are generated&bound
 * It is heavily based off https://www.khronos.org/assets/uploads/developers/library/2014-siggraph-bof/KITE-BOF_Aug14.pdf
 */
void ParticleSystem::initParticles()
{
    colorShader->activate();
    GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

    particleVAO = generateBuffer(*particleModel);

    // Positions
    glGenBuffers(1, &particlePosSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlePosSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct pos), particlePoints, GL_DYNAMIC_COPY);

    particlePoints = (struct pos *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct pos), bufMask);

    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        particlePoints[i].x = rand() / (float)RAND_MAX;
        particlePoints[i].y = rand() / (float)RAND_MAX;
        particlePoints[i].z = rand() / (float)RAND_MAX;
        particlePoints[i].w = 1.0;
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Velocities
    glGenBuffers(1, &particleVelSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleVelSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct vel), particleVels, GL_DYNAMIC_COPY);
    particleVels = (struct vel *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct vel), bufMask);
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        particleVels[i].vx = MAX_VELOCITY * (rand() - rand()) / (float)RAND_MAX;
        particleVels[i].vy = MAX_VELOCITY * (rand() - rand()) / (float)RAND_MAX;
        particleVels[i].vz = MAX_VELOCITY * (rand() - rand()) / (float)RAND_MAX;
        particleVels[i].vw = 0;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Colors
    glGenBuffers(1, &particleColSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleColSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct color), particleCols, GL_DYNAMIC_COPY);
    particleCols = (struct color *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct color), bufMask);
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        particleCols[i].r = 1.0;
        particleCols[i].g = 0.0;
        particleCols[i].b = 0.0;
        particleCols[i].a = 1.0;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    // Set up vertex attribute arrays for compute shader outputs
    glBindBuffer(GL_ARRAY_BUFFER, particlePosSSBO);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribDivisor(3, 1);

    glBindBuffer(GL_ARRAY_BUFFER, particleVelSSBO);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribDivisor(4,1);

    glBindBuffer(GL_ARRAY_BUFFER, particleColSSBO);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribDivisor(5,1);
}