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
        void draw(void);
        void bindFramebuffer(void);
        void unbindFramebuffer(void);
        void renderUI(void);
        void incrementCurrentEffect(void);
        void decrementCurrentEffect(void);
        GLuint color_texture;
        GLuint depth_texture;
        Gloom::Shader *screen_shader;
    private:
        GLuint vao;
        GLuint fb;
        bool has_depth_texture = false;


        ScreenEffect current_effect = ScreenEffect::NORMAL;

        void initFramebuffer(void);
};