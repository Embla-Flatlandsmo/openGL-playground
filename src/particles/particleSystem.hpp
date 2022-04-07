#pragma once

// Local headers
#include <particles/boundingBox.hpp>
#include <utilities/window.hpp>
#include <utilities/camera.hpp>
#include <utilities/shader.hpp>
#include <utilities/mesh.h>

#define NUM_PARTICLES 1024*32

struct pos
{
    float x, y, z, w;
};

struct vel
{
    float vx, vy, vz, vw;
};

struct acc
{
    float ax, ay, az, aw;
};

struct particleProperties
{
    float size = 0.1;
    float cohesion_factor = 0.005;
    float separation_factor = 0.4;
    float separation_range = 0.5;
    float alignment_factor = 0.6;
    float boundary_avoidance_factor = 0.01;
    float dt = 0.01;
    float max_vel = 0.3;
    bool wrap_around = true;
    float view_range = 3.0;
};

class ParticleSystem
{
public:
    ParticleSystem(glm::vec3 boundingBoxLow, glm::vec3 boundingBoxHigh);
    ~ParticleSystem();
    void update();
    void render(GLFWwindow *window, Gloom::Camera *camera);
    void setDebug(bool enable);
    // void setDebugMode(bool debug);
    void resetPositions(void);
    void renderUI(void);
    struct particleProperties boidProperties;

private:
    BoundingBox *boundingBox;
    GLuint particlePosSSBO;
    GLuint particlePosSSBO_prev;
    GLuint particleVelSSBO;
    GLuint particleVelSSBO_prev;
    GLuint particleAccSSBO;
    GLuint particleVAO;

    struct pos *particlePoints;
    struct vel *particleVels;
    struct acc *particleAccs;

    Gloom::Shader *colorShader;
    Gloom::Shader *computeShader;
    Gloom::Shader *forceShader;

    GLuint prefixSumsLoc;
    GLuint bucketSizesLoc;
    GLuint *prefixSums;
    GLuint *bucketSizes;

    Gloom::Shader *prefixSumShader;
    Gloom::Shader *gridBucketsShader;
    Gloom::Shader *reindexShader;

    Mesh *particleModel;

    bool debug = false;

    void initParticles();
    void countBucketSizes();
    void computePrefixSum();
    void computeReindexGrid();
    void initGridSorting();
};