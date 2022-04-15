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
        /**
         * @brief Construct a new Cloud Box object
         * 
         * @param low lower bound of AABB
         * @param high upper bound of AABB
         */
        CloudBox(glm::vec3 low, glm::vec3 high);
        ~CloudBox();

        /**
         * @brief render the clouds
         * 
         */
        void render(void);
        /**
         * @brief Ray march clouds to generate 2D texture for the 
         * CloudBox::render(void) function to use
         */
        void update(Gloom::Camera *camera);

        /**
         * @brief Renders ImGUI panel
         */
        void renderUI(void);

        /**
         * @brief Set the depth texture in the raymarch shader
         * 
         * @param textureID ID of the texture to bind to the depth texture
         */
        void setDepthBuffer(GLuint textureID);
        /**
         * @brief Set the fragColor texture in the raymarch shader
         * 
         * @param textureID ID of the texture to bind to the fragColor texture
         */
        void setColorBuffer(GLuint textureID);

        /**
         * @brief Updates sun direction
         * 
         * @param direction new direction
         */
        void updateSun(glm::vec3 direction);
        /**
         * @brief Set the Debug object
         * 
         * @param enable 
         */
        void setDebug(bool enable);
    private:
        GLuint vao = -1;

        ScreenQuad screen = ScreenQuad(false);

        glm::vec3 boxLow;
        glm::vec3 boxHigh;

        Gloom::Shader* rayMarchCloud;

        Gloom::Shader* perlinWorley;

        GLuint perlinTex = 0;

        bool debug = false;
        /**
         * @brief Parameters for clouds, set in runtime using ImGui
         */
        struct cloudProperties cloud_properties;
};