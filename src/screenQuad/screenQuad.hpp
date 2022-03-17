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

/**
 * @brief Quad class for post-processing effects.
 * 
 */
class ScreenQuad 
{
    public:
        ScreenQuad(bool has_depth_texture);
        ScreenQuad(void); //TODO: this is messy
        ~ScreenQuad(void);
        void draw(void);
        void bindFramebuffer(void);
        void unbindFramebuffer(void);

        void incrementCurrentEffect(void);
        void decrementCurrentEffect(void);
        GLuint color_texture;
        GLuint depth_texture;
    private:
        GLuint vao;
        GLuint fb;
        bool has_depth_texture = false;

        Gloom::Shader *screen_shader;

        ScreenEffect current_effect = ScreenEffect::NORMAL;

        void initFramebuffer(void);
};