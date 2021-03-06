#pragma once
// Local headers
#include <utilities/camera.hpp>
#include <utilities/shader.hpp>
#include <screenQuad/screenQuad.hpp>

// System headers
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


class Sky
{
    public:
        Sky();
        ~Sky();
        void render(void);
        /**
         * @brief Sends necessary uniforms to the sky shader
         * 
         * @param camera camera to use for the shader
         */
        void update(Gloom::Camera *camera);
        void renderUI(void);
    private:
        ScreenQuad* quad;
        glm::vec3 sky_color_bottom = glm::vec3(183/255.0, 237/255.0, 255/255.0);
        glm::vec3 sky_color_top =  glm::vec3(0.0,0.74902, 1.0);
};