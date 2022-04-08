#pragma once
#include "utilities/shader.hpp"
#include <glad/glad.h>

enum ScreenEffect 
{
    NORMAL,
    CHROMATIC_ABERRATION,
    INVERT,
    BLUR,
    SHARPEN,
};

enum QuadUsage
{
    POST_PROCESSING,
    SKY,
};

/**
 * @brief Quad class for post-processing effects.
 * 
 */
class ScreenQuad 
{
    public:
        ScreenQuad(bool has_depth_texture);
        ScreenQuad(QuadUsage usage);
        ScreenQuad(void); //TODO: this is messy
        ~ScreenQuad(void);
        /***/
        void render(void);
        void bindFramebuffer(void);
        void unbindFramebuffer(void);
        void renderUI(void);
        GLuint color_texture;
        GLuint depth_texture;
        Gloom::Shader *screen_shader;
    private:
        /**
         * @brief VAO of the quad covering the screen
         * 
         */
        GLuint vao;
        GLuint fb;

        /**
         * @brief For sky usage, there is no depth texture
         * 
         */
        bool has_depth_texture = false;

        /**
         * @brief Post-processing effect to apply
         */
        ScreenEffect current_effect = ScreenEffect::NORMAL;

        void initFramebuffer(void);
};