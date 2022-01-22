#pragma once
#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <utilities/window.hpp>
#include <utilities/camera.hpp>
void initParticleSystem();
void updateParticles();
void renderParticles(GLFWwindow* window, Gloom::Camera* camera);

#endif // PARTICLE_HPP