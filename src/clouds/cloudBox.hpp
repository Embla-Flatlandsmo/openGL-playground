#pragma once
// Local headers
#include <utilities/camera.hpp>
#include <utilities/shader.hpp>
#include <screenQuad/screenQuad.hpp>

// System headers
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


struct cloudProperties
{
    float step_size = 3.0f;
    float coverage_multiplier = 1.0f;
    float texture_scale = 2.0f;
    float weather_texture_scale = 0.03f;
    float cloud_speed = 10.0f;
    float density_factor = 1.0f;
    float sun_power = 30.0f;
    float fog_factor = 0.75f;

    glm::vec3 cloud_shadow_color = glm::vec3(0.65,0.65,0.75);
    glm::vec3 cloud_light_color =  glm::vec3(1.0,0.6,0.3);
};

class CloudBox 
{
    public:
        CloudBox(glm::vec3 low, glm::vec3 high);
        ~CloudBox();
        void render(Gloom::Camera *camera);
        void renderUI(void);
        void setDepthBuffer(GLuint textureID);
        void setColorBuffer(GLuint textureID);
        void updateSun(glm::vec3 direction);
        void setDebug(bool enable);
    private:
        void generateTextures();
        void generateWeatherMap();
        GLuint vao = -1;

        ScreenQuad screen = ScreenQuad(false);

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

        bool debug = false;
        struct cloudProperties cloud_properties;
};