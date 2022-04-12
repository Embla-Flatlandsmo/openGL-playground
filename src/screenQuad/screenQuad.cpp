#include <screenQuad/screenQuad.hpp>

#include <utilities/window.hpp>
#include <utilities/glutils.h>
#include <utilities/shapes.h>
#include <utilities/texture.hpp>

// #include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <string.h>
#include <iostream>

#include "utilities/imgui/imgui.h"
#include "utilities/imgui/imgui_impl_glfw.h"
#include "utilities/imgui/imgui_impl_opengl3.h"

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

ScreenQuad::ScreenQuad(void)
{
    screen_shader = new Gloom::Shader();
    screen_shader->makeBasicShader("../res/shaders/screen_quad/screen_quad.vert", "../res/shaders/screen_quad/post_process.frag");
    screen_shader->activate();
    this->has_depth_texture = true;
    Mesh screen_quad = generateScreenQuad();
    vao = generateBuffer(screen_quad);
    initFramebuffer();
    glUseProgram(0);
    glBindVertexArray(0);
}

ScreenQuad::ScreenQuad(bool has_depth_texture = true)
{
    screen_shader = new Gloom::Shader();
    screen_shader->makeBasicShader("../res/shaders/screen_quad/screen_quad.vert", "../res/shaders/screen_quad/post_process.frag");
    screen_shader->activate();
    this->has_depth_texture = has_depth_texture;
    Mesh screen_quad = generateScreenQuad();
    vao = generateBuffer(screen_quad);
    initFramebuffer();
    glUseProgram(0);
    glBindVertexArray(0);
}

ScreenQuad::ScreenQuad(QuadUsage usage = POST_PROCESSING)
{
    screen_shader = new Gloom::Shader();
    switch (usage)
    {
        case POST_PROCESSING:
            screen_shader->makeBasicShader("../res/shaders/screen_quad/screen_quad.vert", "../res/shaders/screen_quad/post_process.frag");
            break;
        case SKY:
            screen_shader->makeBasicShader("../res/shaders/screen_quad/screen_quad.vert", "../res/shaders/screen_quad/sky.frag");
            break;
        default:
            screen_shader->makeBasicShader("../res/shaders/screen_quad/screen_quad.vert", "../res/shaders/screen_quad/post_process.frag");
            break;
    }

    screen_shader->activate();
    this->has_depth_texture = false;
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
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    // make sure we clear the framebuffer's content
    // glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ScreenQuad::unbindFramebuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/**
 * @brief 
 * 
 */
void ScreenQuad::render(void)
{
    screen_shader->activate();
    glUniform1ui(screen_shader->getUniformFromName("effect"), current_effect);
    glBindVertexArray(vao);
    glDisable(GL_DEPTH_TEST);
    glBindTextureUnit(0, color_texture);
    if (has_depth_texture)
    {
        glBindTextureUnit(1, depth_texture);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    glEnable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    return;
}

/**
 * @brief Renders a box for selection of 
 * post-processing effects
 */
void ScreenQuad::renderUI(void)
{
    ImGui::Begin("Post Processing Effect");
    const char* items[] = { "Normal", "Chromatic Aberration", "Inverted", "Blurred", "Sharpened" };
    static int item_current = 0;
    ImGui::Combo("Effect", &item_current, items, IM_ARRAYSIZE(items));
    current_effect = (ScreenEffect)item_current;
    ImGui::End();
}

/**
 * @brief Helper function for initializing the framebuffers.
 * Most of the code is lifted from https://learnopengl.com/Advanced-OpenGL/Framebuffers 
 */
void ScreenQuad::initFramebuffer(void)
{
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    // ------ THE RENDER TEXTURE ------
    color_texture = generateTexture2D(windowWidth, windowHeight);

    // ----- THE DEPTH TEXTURE --------
    if (has_depth_texture)
    {
        glGenTextures(1, &depth_texture);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);
    }
    else 
    {
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); 
        // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture, 0);
    GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer not initialized correctly!");
        throw "Framebuffer not initialized correctly!";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}