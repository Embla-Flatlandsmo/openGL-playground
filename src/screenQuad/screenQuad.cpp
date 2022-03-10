#include <screenQuad/screenQuad.hpp>

#include <utilities/window.hpp>
#include <utilities/glutils.h>
#include <utilities/shapes.h>

// #include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <string.h>
#include <iostream>

std::string stringFromEnum(ScreenEffect effect) {
    switch(effect){
        case NORMAL:
            return "normal";
        case CHROMATIC_ABERRATION:
            return "chromatic aberration";
        case INVERT:
            return "inverted";
        case BLUR:
            return "blurred";
        case SHARPEN:
            return "sharpened";
        default:
            return "";
    }

}
void ScreenQuad::incrementCurrentEffect(void)
{
    // The fancy operator is just so that the enum wraps around to NORMAL if you try to increment past the SHARPEN effect.
    current_effect = (current_effect == ScreenEffect::SHARPEN) ? ScreenEffect::NORMAL : static_cast<ScreenEffect>(static_cast<int>(current_effect)+1);
    std::cout << "Effect: " << stringFromEnum(current_effect) << std::endl;
}

void ScreenQuad::decrementCurrentEffect(void)
{
    current_effect = (current_effect == ScreenEffect::NORMAL) ? ScreenEffect::SHARPEN : static_cast<ScreenEffect>(static_cast<int>(current_effect)-1);
    std::cout << "Effect: " << stringFromEnum(current_effect) << std::endl;
}

ScreenQuad::ScreenQuad()
{
    screen_shader = new Gloom::Shader();
    screen_shader->makeBasicShader("../res/shaders/screen.vert", "../res/shaders/screen.frag");
    screen_shader->activate();

    Mesh screen_quad = generateScreenQuad();
    vao = generateBuffer(screen_quad);
    initFramebuffer();
    glUseProgram(0);
    glBindVertexArray(0);
}

ScreenQuad::~ScreenQuad()
{
    glDeleteFramebuffers(1, &fb);
}

void ScreenQuad::bindFramebuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glEnable(GL_DEPTH_TEST);
    // make sure we clear the framebuffer's content
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ScreenQuad::unbindFramebuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);
}

void ScreenQuad::draw(void)
{
    screen_shader->activate();
    glUniform1ui(screen_shader->getUniformFromName("effect"), current_effect);
    glBindVertexArray(vao);
    glDisable(GL_DEPTH_TEST);
    glBindTextureUnit(0, texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    glEnable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    return;
}

/**
 * @brief Helper function for initializing the framebuffers.
 * Most of the code is lifted from https://learnopengl.com/Advanced-OpenGL/Framebuffers 
 */
void ScreenQuad::initFramebuffer(void)
{
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    // create a color attachment texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw "Framebuffer not initialized correctly!";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}