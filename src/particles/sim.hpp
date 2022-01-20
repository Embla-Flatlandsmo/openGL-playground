#ifndef SIM_HPP
#define SIM_HPP
#include <cstddef>
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <iostream>
#include <GLFW/glfw3.h>
#include <utilities/camera.hpp>

/** @brief VAO for boids-related buffers */
static GLuint vao;

/** @brief Buffer with boid array copied from CPU */
static GLuint bufAgents;

/** @brief Buffer with boid geometry */
static GLuint bufGeometry;

/** @brief Buffer with indicies for boid geometry */
static GLuint ebufGeometry;

/** @brief Size of boid object */
static float boidSize = 2.0;

void initShaders();
void initBuffers();
void drawBoids(GLFWwindow* window, Gloom::Camera* camera);

#endif