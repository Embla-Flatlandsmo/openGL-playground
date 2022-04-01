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
    sky = new ScreenQuad(SKY);
}
Sky::~Sky()
{
    free(sky);
}

void Sky::render(Gloom::Camera *camera)
{
    sky->screen_shader->activate();
    glUniformMatrix4fv(sky->screen_shader->getUniformFromName("view_mat"), 1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));
    glUniformMatrix4fv(sky->screen_shader->getUniformFromName("inv_proj"), 1, GL_FALSE, glm::value_ptr(glm::inverse(camera->getProjMatrix())));
    glUniformMatrix4fv(sky->screen_shader->getUniformFromName("inv_view"), 1, GL_FALSE, glm::value_ptr(glm::inverse(camera->getViewMatrix())));
    sky->draw();
}

void Sky::renderUI(void)
{
    ImGui::Begin("Sky properties");
    // ImGui::SliderFloat("Step size", &step_size, 0.5f, 3.f);
    ImGui::End();
}