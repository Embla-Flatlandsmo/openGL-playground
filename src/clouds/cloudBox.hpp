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
    float light_step_size = 3.0f;
    float texture_scale = 2.1f;
    float weather_texture_scale = 1.5f;
    float cloud_speed = 10.0f;
    float density_factor = 1.0f;
    float sun_power = 190.0f;
    float fog_factor = 1.0f;

    glm::vec3 cloud_shadow_color = glm::vec3(0.65,0.65,0.75);
    glm::vec3 cloud_light_color =  glm::vec3(1.0,0.6,0.3);
};

class CloudBox 
{
    public:
        CloudBox(glm::vec3 low, glm::vec3 high);
        ~CloudBox();
        void render(void);
        void update(Gloom::Camera *camera);
        void renderUI(void);
        void setDepthBuffer(GLuint textureID);
        void setColorBuffer(GLuint textureID);
        void updateSun(glm::vec3 direction);
        void setDebug(bool enable);
    private:
        void generateTextures();
        GLuint vao = -1;

        ScreenQuad screen = ScreenQuad(false);

        glm::vec3 boxLow;
        glm::vec3 boxHigh;

        Gloom::Shader* rayMarchCloud;

        Gloom::Shader* perlinWorley;

        GLuint perlinTex = 0;

        bool debug = false;
        struct cloudProperties cloud_properties;
};