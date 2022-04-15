#include <sky/sky.hpp>

// System headers
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>


#include "utilities/imgui/imgui.h"
#include "utilities/imgui/imgui_impl_glfw.h"
#include "utilities/imgui/imgui_impl_opengl3.h"

Sky::Sky()
{
    quad = new ScreenQuad(SKY);
}
Sky::~Sky()
{
    // TODO: be a good programmer and destroy this properly
    quad->screen_shader->destroy();
}

void Sky::render()
{
    quad->render();
}

void Sky::update(Gloom::Camera *camera)
{
    quad->screen_shader->activate();
    glUniformMatrix4fv(quad->screen_shader->getUniformFromName("view_mat"), 1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));
    glUniformMatrix4fv(quad->screen_shader->getUniformFromName("inv_proj"), 1, GL_FALSE, glm::value_ptr(glm::inverse(camera->getProjMatrix())));
    glUniformMatrix4fv(quad->screen_shader->getUniformFromName("inv_view"), 1, GL_FALSE, glm::value_ptr(glm::inverse(camera->getViewMatrix())));
}

void Sky::renderUI(void)
{
    // Not implemented
}