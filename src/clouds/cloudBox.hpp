#pragma once
// Local headers
#include <utilities/camera.hpp>
#include <utilities/shader.hpp>

// System headers
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class CloudBox 
{
    public:
        CloudBox(glm::vec3 low, glm::vec3 high);
        ~CloudBox();
        void render(Gloom::Camera *camera);
    private:
        GLuint vao = -1;
        glm::vec3 boxLow;
        glm::vec3 boxHigh;

        Gloom::Shader* renderCloud;
};