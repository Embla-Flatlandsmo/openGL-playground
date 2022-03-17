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

ScreenQuad::ScreenQuad(void)
{
    screen_shader = new Gloom::Shader();
    screen_shader->makeBasicShader("../res/shaders/screen.vert", "../res/shaders/screen.frag");
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
    screen_shader->makeBasicShader("../res/shaders/screen.vert", "../res/shaders/screen.frag");
    screen_shader->activate();
    this->has_depth_texture = has_depth_texture;
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ScreenQuad::unbindFramebuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ScreenQuad::draw(void)
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
 * @brief Helper function for initializing the framebuffers.
 * Most of the code is lifted from https://learnopengl.com/Advanced-OpenGL/Framebuffers 
 */
void ScreenQuad::initFramebuffer(void)
{
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    // create a color attachment color_texture
    // glGenTextures(1, &color_texture);
    // glBindTexture(GL_TEXTURE_2D, color_texture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture, 0);



    // glGenTextures(1, &depth_texture);
    // glBindTexture(GL_TEXTURE_2D, depth_texture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, depth_texture, 0);
    // glBindTexture(GL_TEXTURE_2D, 0);

    // glGenTextures(1, &depth_texture);
    // glBindTexture(GL_TEXTURE_2D, depth_texture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    // // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    // // glTexImage2D(
    // //     GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, windowWidth, windowHeight, 0, 
    // //     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL
    // // );
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    // glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    

    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);


    // unsigned int rbo;
    // glGenRenderbuffers(1, &rbo);
    // glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, windowWidth, windowHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 
   // ------ THE RENDER TEXTURE ------
   color_texture = generateTexture2D(windowWidth, windowHeight);
    // glGenTextures(1, &color_texture);
    // glBindTexture(GL_TEXTURE_2D, color_texture);
    // // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (has_depth_texture)
    {
        glGenTextures(1, &depth_texture);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT24, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
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
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); 
    }


    // ----- THE DEPTH TEXTURE --------
    // glGenTextures(1, &depth_texture);
    // glBindTexture(GL_TEXTURE_2D, depth_texture);
    // // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // create the depth buffer
    // unsigned int rbo;
    // glGenRenderbuffers(1, &rbo);
    // glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, windowWidth, windowHeight); //512, 512);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);


    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture, 0);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, depth_texture, 0);
    GLuint attachments[1] = { GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer not initialized correctly!");
        throw "Framebuffer not initialized correctly!";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}