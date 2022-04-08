// Local headers
#include "particleSystem.hpp"
#include <utilities/shader.hpp>
#include <particles/boundingBox.hpp>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <utilities/objLoader.hpp>
#include <utilities/timeutils.h>
// System headers
#include <glad/glad.h>
#include <math.h>       /* ceil */
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "utilities/imgui/imgui.h"
#include "utilities/imgui/imgui_impl_glfw.h"
#include "utilities/imgui/imgui_impl_opengl3.h"

#define WORK_GROUP_SIZE 1024

ParticleSystem::ParticleSystem(glm::vec3 low, glm::vec3 high)
{
    particleModel = new Mesh(loadObj("../res/models/boid.obj"));

    boundingBox = new BoundingBox(low, high, boidProperties.view_range);

    particlePoints = new struct pos[NUM_PARTICLES];
    particleVels = new struct vel[NUM_PARTICLES];
    particleAccs = new struct acc[NUM_PARTICLES];

    renderShader = new Gloom::Shader();
    renderShader->makeBasicShader("../res/shaders/boids/particle.vert", "../res/shaders/boids/particle.frag");

    // The compute shader must be in its own program
    integrationShader = new Gloom::Shader();
    integrationShader->attach("../res/shaders/boids/integrate.comp");
    integrationShader->link();

    // We set the uniforms for the bounding box in the compute shader only once
    integrationShader->activate();
    glUniform3fv(integrationShader->getUniformFromName("boundingBoxLow"), 1, glm::value_ptr(boundingBox->low));
    glUniform3fv(integrationShader->getUniformFromName("boundingBoxHigh"), 1, glm::value_ptr(boundingBox->high));
    glUniform1i(integrationShader->getUniformFromName("numBoids"), NUM_PARTICLES);
    glUniform3uiv(integrationShader->getUniformFromName("gridRes"), 1, glm::value_ptr(boundingBox->resolution));
    integrationShader->deactivate();

    forceShader = new Gloom::Shader();
    forceShader->attach("../res/shaders/boids/computeForce.comp");
    forceShader->link();

    // We set the uniforms for the bounding box in the compute shader only once
    forceShader->activate();
    glUniform3fv(forceShader->getUniformFromName("boundingBoxLow"), 1, glm::value_ptr(boundingBox->low));
    glUniform3fv(forceShader->getUniformFromName("boundingBoxHigh"), 1, glm::value_ptr(boundingBox->high));
    glUniform1i(forceShader->getUniformFromName("numBoids"), NUM_PARTICLES);
    glUniform3uiv(forceShader->getUniformFromName("gridRes"), 1, glm::value_ptr(boundingBox->resolution));
    glUniform1f(forceShader->getUniformFromName("view_range"), boidProperties.view_range);
    forceShader->deactivate();

    initParticles();
    initGridSorting();
}

ParticleSystem::~ParticleSystem()
{
    // Some cleanup here is probably needed...
    integrationShader->destroy();
    renderShader->destroy();
}

void ParticleSystem::update()
{
    // getTimeDeltaSeconds();
    countBucketSizes();
    // printf("Bucket sizes time: %f\n", getTimeDeltaSeconds());
    computePrefixSum();
    // printf("Prefix sum time: %f\n", getTimeDeltaSeconds());
    computeReindexGrid();
    // printf("Reindex grid time: %f\n", getTimeDeltaSeconds());

    forceShader->activate();
    glUniform1f(forceShader->getUniformFromName("cohesion_factor"), boidProperties.cohesion_factor);
    glUniform1f(forceShader->getUniformFromName("separation_range"), boidProperties.separation_range);
    glUniform1f(forceShader->getUniformFromName("separation_factor"), boidProperties.separation_factor);
    glUniform1f(forceShader->getUniformFromName("alignment_factor"), boidProperties.alignment_factor);
    glUniform1f(forceShader->getUniformFromName("boundary_avoidance_factor"), boidProperties.boundary_avoidance_factor);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particlePosSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleVelSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particleAccSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, prefixSumsLoc);

    glDispatchCompute(ceil(float(NUM_PARTICLES)/WORK_GROUP_SIZE), 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    forceShader->deactivate();
    // printf("Force update time: %f\n", getTimeDeltaSeconds());

    integrationShader->activate();
    glUniform1f(integrationShader->getUniformFromName("DT"), boidProperties.dt);
    glUniform1f(integrationShader->getUniformFromName("max_vel"), boidProperties.max_vel);
    glUniform1i(integrationShader->getUniformFromName("wrap_around"), boidProperties.wrap_around);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particlePosSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleVelSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particleAccSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, prefixSumsLoc);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, bucketSizesLoc);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, particlePosSSBO_prev);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, particleVelSSBO_prev);  
    glDispatchCompute(ceil(float(NUM_PARTICLES)/WORK_GROUP_SIZE), 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    integrationShader->deactivate();
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
    glm::mat4 projection = camera->getProjMatrix();
    glm::mat4 VP = projection * camera->getViewMatrix();

    if (debug)
    {
        boundingBox->renderAsWireframe(window, camera);
    }
    renderShader->activate();
    // Update uniforms
    glUniform1f(renderShader->getUniformFromName("particle_size"), boidProperties.size);
    glUniform1f(renderShader->getUniformFromName("fog_factor"), boidProperties.fog_factor);
    glUniformMatrix4fv(renderShader->getUniformFromName("VP"), 1, GL_FALSE, glm::value_ptr(VP));
    glUniform3fv(renderShader->getUniformFromName("camera_pos"), 1, glm::value_ptr(glm::vec3(camera->getViewMatrix()[3])));
    glBindVertexArray(particleVAO);
    // Finally we draw the particles
    // glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
    glDrawElementsInstanced(GL_TRIANGLES, particleModel->indices.size(), GL_UNSIGNED_INT, (void *)0, NUM_PARTICLES);
    renderShader->deactivate();
}

void ParticleSystem::setDebug(bool enable)
{
    debug = enable;
}

void ParticleSystem::renderUI(void)
{
    if (debug)
    {
        ImGui::Begin("Boid properties");
        ImGui::SliderFloat("Cohesion", &boidProperties.cohesion_factor, 0.0f, 1.5f);
        ImGui::SliderFloat("Alignment", &boidProperties.alignment_factor, 0.0f, 1.5f);
        ImGui::SliderFloat("Separation", &boidProperties.separation_factor, 0.0f, 1.5f);
        ImGui::SliderFloat("Separation Range", &boidProperties.separation_range, 0.0f, 3.0f); // Max is view range
        ImGui::SliderFloat("Boundary avoidance", &boidProperties.boundary_avoidance_factor, 0.0f, 0.2f);
        ImGui::SliderFloat("dt", &boidProperties.dt, 0.0f, 2.0);
        ImGui::SliderFloat("Max velocity", &boidProperties.max_vel, 0.0f, 4.0f);
        ImGui::Checkbox("Wrap around", &boidProperties.wrap_around);
        ImGui::Separator();

        ImGui::SliderFloat("Size", &boidProperties.size, 0.01f, 0.3f);
        ImGui::SliderFloat("fog_factor", &boidProperties.fog_factor, 0.0f, 0.03f);
        ImGui::End();
    }
}

/**
 * @brief Helper function for initializing the particles.
 * This is where VAO is created and the SSBOs are generated&bound
 * It is heavily based off https://www.khronos.org/assets/uploads/developers/library/2014-siggraph-bof/KITE-BOF_Aug14.pdf
 */
void ParticleSystem::initParticles()
{
    renderShader->activate();
    GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

    particleVAO = generateBuffer(*particleModel);

    // Positions
    uint32_t positions_size = NUM_PARTICLES * sizeof(struct pos);
    glGenBuffers(1, &particlePosSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlePosSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct pos), particlePoints, GL_DYNAMIC_COPY);

    particlePoints = (struct pos *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, positions_size, bufMask);
    glm::vec3 diagonal = boundingBox->high-boundingBox->low;
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        particlePoints[i].x = boundingBox->low.x+diagonal.x*(rand() / (float)RAND_MAX);
        particlePoints[i].y = boundingBox->low.y+diagonal.y*(rand() / (float)RAND_MAX);
        particlePoints[i].z = boundingBox->low.z+diagonal.z*(rand() / (float)RAND_MAX);
        particlePoints[i].w = 1.0;
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    
    // Prev positions
    glGenBuffers(1, &particlePosSSBO_prev);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlePosSSBO_prev);
    glBufferData(GL_SHADER_STORAGE_BUFFER, positions_size, nullptr, GL_DYNAMIC_COPY);

    // Velocities
    uint32_t velocities_size = NUM_PARTICLES*sizeof(struct vel);
    glGenBuffers(1, &particleVelSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleVelSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, velocities_size, particleVels, GL_DYNAMIC_COPY);
    particleVels = (struct vel *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, velocities_size, bufMask);
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        particleVels[i].vx = boidProperties.max_vel * (rand() - rand()) / (float)RAND_MAX;
        particleVels[i].vy = boidProperties.max_vel * (rand() - rand()) / (float)RAND_MAX;
        particleVels[i].vz = boidProperties.max_vel * (rand() - rand()) / (float)RAND_MAX;
        particleVels[i].vw = 0;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


    // previous velocity
    glGenBuffers(1, &particleVelSSBO_prev);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleVelSSBO_prev);
    glBufferData(GL_SHADER_STORAGE_BUFFER, velocities_size, nullptr, GL_DYNAMIC_COPY);

    // Accelerations
    glGenBuffers(1, &particleAccSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleAccSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct acc), particleAccs, GL_DYNAMIC_COPY);
    particleAccs = (struct acc *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct acc), bufMask);
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        particleAccs[i].ax = 0.0;
        particleAccs[i].ay = 0.0;
        particleAccs[i].az = 0.0;
        particleAccs[i].aw = 1.0;
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

    glBindBuffer(GL_ARRAY_BUFFER, particleAccSSBO);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribDivisor(5,1);
}


void ParticleSystem::initGridSorting() 
{
    prefixSumShader = new Gloom::Shader();
    prefixSumShader->attach("../res/shaders/boids/prefixSum.comp");
    prefixSumShader->link();

    gridBucketsShader = new Gloom::Shader();
    gridBucketsShader->attach("../res/shaders/boids/gridBuckets.comp");
    gridBucketsShader->link();
    gridBucketsShader->activate();
    glUniform3fv(gridBucketsShader->getUniformFromName("boundingBoxLow"), 1, glm::value_ptr(boundingBox->low));
    glUniform3fv(gridBucketsShader->getUniformFromName("boundingBoxHigh"), 1, glm::value_ptr(boundingBox->high));
    glUniform3uiv(gridBucketsShader->getUniformFromName("gridRes"), 1, glm::value_ptr(boundingBox->resolution));
    glUniform1ui(gridBucketsShader->getUniformFromName("numBoids"), NUM_PARTICLES);
    gridBucketsShader->deactivate();
    
    reindexShader = new Gloom::Shader();
    reindexShader->attach("../res/shaders/boids/reindex.comp");
    reindexShader->link();
    reindexShader->activate();
    glUniform3fv(reindexShader->getUniformFromName("boundingBoxLow"), 1, glm::value_ptr(boundingBox->low));
    glUniform3fv(reindexShader->getUniformFromName("boundingBoxHigh"), 1, glm::value_ptr(boundingBox->high));
    glUniform3uiv(reindexShader->getUniformFromName("gridRes"), 1, glm::value_ptr(boundingBox->resolution));
    glUniform1ui(reindexShader->getUniformFromName("numBoids"), NUM_PARTICLES);
    reindexShader->deactivate();


    GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
    const uint32_t numCells = boundingBox->numCells;
    prefixSums = new GLuint[numCells];
    bucketSizes = new GLuint[numCells];

    glGenBuffers(1, &prefixSumsLoc);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, prefixSumsLoc);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numCells * sizeof(GLuint), prefixSums, GL_DYNAMIC_COPY);
    prefixSums = (GLuint*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numCells * sizeof(GLuint), bufMask);
    for (uint32_t i = 0; i < numCells; i++)
    {
        prefixSums[i]=0;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glGenBuffers(1, &bucketSizesLoc);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bucketSizesLoc);
    glBufferData(GL_SHADER_STORAGE_BUFFER, numCells * sizeof(GLuint), bucketSizes, GL_DYNAMIC_COPY);
    bucketSizes = (GLuint*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numCells * sizeof(GLuint), bufMask);
    for (uint32_t i = 0; i < numCells; i++)
    {
        bucketSizes[i]=0;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}


void ParticleSystem::countBucketSizes() {
    gridBucketsShader->activate();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particlePosSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, bucketSizesLoc);
    glDispatchCompute(ceil(float(NUM_PARTICLES)/WORK_GROUP_SIZE), 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    gridBucketsShader->deactivate();

    /* For debugging :) */
    // uint32_t arraySize = sizeof(GLuint)*(boundingBox->numCells);
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, bucketSizesLoc);
    // GLuint* sizes = (GLuint*)glMapNamedBufferRange(bucketSizesLoc, 0, arraySize, GL_MAP_READ_BIT);
    // memcpy(bucketSizes, sizes, arraySize);
    // glUnmapNamedBuffer(bucketSizesLoc);
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    // int sum;
    // for (int i = 0; i < boundingBox->numCells; i++) {
    //         sum += bucketSizes[i];
    //     if (bucketSizes[i] > NUM_PARTICLES) {
    //         printf("Bucket size at [%i] is %i\n", i, bucketSizes[i]);
    //     }
    // }
    // printf("BucketSizes[100]=%i\n", bucketSizes[100]);
    // printf("Sum of all buckets: %i", sum);
}

void ParticleSystem::computePrefixSum() {

    prefixSumShader->activate();

    glUniform1ui(prefixSumShader->getUniformFromName("numCells"), boundingBox->numCells);
    uint32_t chunkSize = 2;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, prefixSumsLoc);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, bucketSizesLoc);
    glUniform1ui(prefixSumShader->getUniformFromName("chunkSize"), chunkSize);
    glDispatchCompute(ceil((float(boundingBox->numCells))/WORK_GROUP_SIZE), 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    // do
    // {
    //     glUniform1ui(prefixSumShader->getUniformFromName("chunkSize"), chunkSize);
    //     glDispatchCompute(ceil((float(boundingBox->numCells))/WORK_GROUP_SIZE), 1, 1);
    //     glMemoryBarrier(GL_ALL_BARRIER_BITS);
    //     chunkSize *= 2;
    // } while (chunkSize <= boundingBox->numCells);

    // // Noe er galt med prefix sum!!!
    // // Fungerer for numCells=4096
    // uint32_t arraySize = sizeof(GLuint)*(boundingBox->numCells);
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, prefixSumsLoc);
    // GLuint* sums = (GLuint*)glMapNamedBufferRange(prefixSumsLoc, 0, arraySize, GL_MAP_READ_BIT);
    // memcpy(prefixSums, sums, arraySize);
    // glUnmapNamedBuffer(prefixSumsLoc);
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // printf("PrefixSum[%d]: %d\n",boundingBox->numCells-1,prefixSums[boundingBox->numCells-1]);
    // printf("Num particles: %i\n", NUM_PARTICLES);
    // prefixSumShader->deactivate();
    
}

void ParticleSystem::computeReindexGrid() {
    reindexShader->activate();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particlePosSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleVelSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, prefixSumsLoc);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, bucketSizesLoc);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, particlePosSSBO_prev);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, particleVelSSBO_prev);    

    glDispatchCompute(ceil(float(NUM_PARTICLES)/WORK_GROUP_SIZE), 1, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    // uint32_t arraySize = NUM_PARTICLES * sizeof(GLuint);
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleIndicesLoc);
    // GLuint* sums = (GLuint*)glMapNamedBufferRange(particleIndicesLoc, 0, arraySize, GL_MAP_READ_BIT);
    // memcpy(particleIndices, sums, arraySize);

    // glUnmapNamedBuffer(particleIndicesLoc);
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // printf("particleIndices[%d]: %d\n",NUM_PARTICLES-1,particleIndices[NUM_PARTICLES-1]);
    // printf("Num particles: %i\n", NUM_PARTICLES);


    reindexShader->deactivate();
} 

void ParticleSystem::resetPositions(void)
{
    GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

    // Positions
    uint32_t positions_size = NUM_PARTICLES * sizeof(struct pos);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlePosSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct pos), particlePoints, GL_DYNAMIC_COPY);

    particlePoints = (struct pos *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, positions_size, bufMask);
    glm::vec3 diagonal = boundingBox->high-boundingBox->low;
    for (int i = 0; i < NUM_PARTICLES; i++)
    {
        particlePoints[i].x = boundingBox->low.x+diagonal.x*(rand() / (float)RAND_MAX);
        particlePoints[i].y = boundingBox->low.y+diagonal.y*(rand() / (float)RAND_MAX);
        particlePoints[i].z = boundingBox->low.z+diagonal.z*(rand() / (float)RAND_MAX);
        particlePoints[i].w = 1.0;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}