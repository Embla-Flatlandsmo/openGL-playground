#pragma once

// Local headers
#include <particles/boundingBox.hpp>
#include <utilities/window.hpp>
#include <utilities/camera.hpp>
#include <utilities/shader.hpp>
#include <utilities/mesh.h>

#define NUM_PARTICLES 1024*64

/**
 * @brief Position data for one particle.
 * Note: only xyz is used, w is there so std140 layout can be used
 * in compute shader
 */
struct pos
{
    float x, y, z, w;
};

/**
 * @brief Velocity data for one particle.
 * Note: only xyz is used, w is there so std140 layout can be used
 * in compute shader
 */
struct vel
{
    float vx, vy, vz, vw;
};

/**
 * @brief Acceleration data for one particle.
 * Note: only xyz is used, w is there so std140 layout can be used
 * in compute shader
 */
struct acc
{
    float ax, ay, az, aw;
};

struct particleProperties
{
    float size = 0.05;
    float cohesion_factor = 0.4;
    float separation_factor = 0.4;
    float separation_range = 0.5;
    float alignment_factor = 0.6;
    float boundary_avoidance_factor = 0.003;
    float dt = 0.3;
    float max_vel = 0.3;
    bool wrap_around = true;
    float view_range = 3.0;

    float fog_factor = 0.01;
};


/**
 * @brief Simulates particles inside a bounding box.
 * 
 */
class ParticleSystem
{
public:
    /**
     * @brief Construct a new Particle System object
     * 
     * @param boundingBoxLow Lower bound for bounding box
     * @param boundingBoxHigh Upper bound for bounding box
     */
    ParticleSystem(glm::vec3 boundingBoxLow, glm::vec3 boundingBoxHigh);
    ~ParticleSystem();
    /**
     * @brief updates velocity and acceleration for each boid
     * 
     */
    void update();
    /**
     * @brief Updates 
     * 
     * @param window 
     * @param camera 
     */
    void render(Gloom::Camera *camera);
    void setDebug(bool enable);
    void resetPositions(void);
    void renderUI(void);


private:

    bool debug = false;
    Mesh *particleModel;

    BoundingBox *boundingBox;
    GLuint particlePosSSBO;
    GLuint particleVelSSBO;
    GLuint particleAccSSBO;
    GLuint particleVAO;

    /**
     * @brief 
     * 
     */
    struct pos *particlePoints;
    struct vel *particleVels;
    struct acc *particleAccs;

    struct particleProperties boidProperties;

    Gloom::Shader *renderShader;
    Gloom::Shader *integrationShader;
    Gloom::Shader *forceShader;

    void initParticles();

    // Necessities for grid-based
    // neighbour check
    GLuint particlePosSSBO_prev;
    GLuint particleVelSSBO_prev;

    GLuint prefixSumsLoc;
    GLuint bucketSizesLoc;
    GLuint *prefixSums;
    GLuint *bucketSizes;

    Gloom::Shader *prefixSumShader;
    Gloom::Shader *gridBucketsShader;
    Gloom::Shader *reindexShader;

    void countBucketSizes();
    void computePrefixSum();
    void computeReindexGrid();
    void initGridSorting();
};