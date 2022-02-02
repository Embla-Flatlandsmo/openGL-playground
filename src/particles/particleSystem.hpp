#pragma once

// Local headers
#include <particles/boundingBox.hpp>
#include <utilities/window.hpp>
#include <utilities/camera.hpp>
#include <utilities/shader.hpp>
#include <utilities/mesh.h>

#define NUM_PARTICLES 1024*2
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

class ParticleSystem
{
public:
    ParticleSystem(glm::vec3 boundingBoxLow, glm::vec3 boundingBoxHigh);
    ~ParticleSystem();
    void update();
    void render(GLFWwindow *window, Gloom::Camera *camera);
    void setDebugMode(bool debug);

private:
    BoundingBox* boundingBox;
    GLuint particlePosSSBO;
    GLuint particleVelSSBO;
    GLuint particleColSSBO;
    GLuint particleVAO;

    struct pos *particlePoints;
    struct vel *particleVels;
    struct color *particleCols;

    Gloom::Shader *colorShader;
    Gloom::Shader *computeShader;

    Mesh* particleModel;
    float particleSize;

    bool debug = true;

    void initParticles();
};