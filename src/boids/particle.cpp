#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <utilities/shader.hpp>
#include "particle.hpp"
#include "boundingBox.hpp"

#define NUM_PARTICLES 1024 * 1024
#define WORK_GROUP_SIZE 128

struct pos
{
    float x, y, z, w;
};

struct vel
{
    float vx, vy, vz, vw;
};

struct color
{
    float r, g, b, a;
};

struct pos *points;
struct vel *vels;
struct color *cols;

GLuint posSSBO;
GLuint velSSBO;
GLuint colSSBO;
GLuint vao;

Gloom::Shader *colorShader;
Gloom::Shader *computeShader;

BoundingBox* boundingBox;

void initParticles()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &posSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct pos), points, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0,  0);

    GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
    points = (struct pos *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct pos), bufMask);

    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        points[i].x = rand() / (float)RAND_MAX;
        points[i].y = rand() / (float)RAND_MAX;
        points[i].z = rand() / (float)RAND_MAX;
        points[i].w = 1.0;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glGenBuffers(1, &velSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct vel), vels, GL_STATIC_DRAW);
    vels = (struct vel *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct vel), bufMask);
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        vels[i].vx = rand() / (float)RAND_MAX;
        vels[i].vy = rand() / (float)RAND_MAX;
        vels[i].vz = rand() / (float)RAND_MAX;
        vels[i].vw = 0;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glGenBuffers(1, &colSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct color), cols, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
    cols = (struct color *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct color), bufMask);
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        cols[i].r = 1.0;
        cols[i].g = 0.0;
        cols[i].b = 0.0;
        cols[i].a = 1.0;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void initParticleSystem()
{
    boundingBox = new BoundingBox(glm::vec3(-20., -20., -20.), glm::vec3(20., 20., 20.));
    colorShader = new Gloom::Shader();
    colorShader->makeBasicShader("res/shaders/particle.vert", "res/shaders/particle.frag");

    computeShader = new Gloom::Shader();
    computeShader->attach("res/shaders/particle.comp");
    computeShader->link();
    // computeShader->activate();

    initParticles();
}

void updateParticles()
{
    computeShader->activate();
    // glm::vec3 boundingBoxLow = glm::vec3(-20.,-20.,-20.);
    // glm::vec3 boundingBoxHigh = glm::vec3(20., 20., 20.);
    // glUniform3fv(computeShader->getUniformFromName("boundingBoxLow"), 1, glm::value_ptr(boundingBoxLow));
    // glUniform3fv(computeShader->getUniformFromName("boundingBoxHigh"), 2, glm::value_ptr(boundingBoxHigh));
    glUniform3fv(computeShader->getUniformFromName("boundingBoxLow"), 1, glm::value_ptr(boundingBox->low));
    glUniform3fv(computeShader->getUniformFromName("boundingBoxHigh"), 1, glm::value_ptr(boundingBox->high));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, posSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, velSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, colSSBO);

    glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, 0);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, 0);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0);
    computeShader->deactivate();
}

void renderParticles(GLFWwindow *window, Gloom::Camera *camera)
{
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
    glm::mat4 VP = projection * camera->getViewMatrix();
    boundingBox->renderAsWireframe(window, camera);
    colorShader->activate();

    glUniformMatrix4fv(colorShader->getUniformFromName("VP"), 1, GL_FALSE, glm::value_ptr(VP));
    glBindVertexArray(vao);
    // Input position values
    glBindBuffer(GL_ARRAY_BUFFER, posSSBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    // Input color values
    glBindBuffer(GL_ARRAY_BUFFER, colSSBO);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);


    glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
    // glDrawElements(GL_POINTS, NUM_PARTICLES, GL_UNSIGNED_INT, nullptr);
    // glDisableVertexAttribArray(0);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glEnable(GL_CULL_FACE);
    colorShader->deactivate();
}