#pragma once
// Local headers
#include <utilities/camera.hpp>
#include <utilities/shader.hpp>
#include <screenQuad/screenQuad.hpp>

// System headers
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class CloudBox 
{
    public:
        CloudBox(glm::vec3 low, glm::vec3 high);
        ~CloudBox();
        void render(Gloom::Camera *camera);
        void renderUI(void);
        void setDepthBuffer(GLuint textureID);
    private:
        void generateTextures();
        void generateWeatherMap();
        GLuint vao = -1;

        ScreenQuad screen;

        glm::vec3 boxLow;
        glm::vec3 boxHigh;

        Gloom::Shader* renderCloud;
        Gloom::Shader* rayMarchCloud;

        Gloom::Shader* perlinWorley;
        Gloom::Shader* worley;
        Gloom::Shader* weather;

        GLuint perlinTex = 0;
        GLuint worley32 = 0;
        GLuint weatherTex = 0;

        float step_size = 0.5f;
        float texture_scale = 1.0f;
};