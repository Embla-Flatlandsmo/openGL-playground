// Local headers
#include "cloudBox.hpp"
#include <utilities/shader.hpp>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <utilities/window.hpp>
#include <utilities/camera.hpp>


// System headers
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

CloudBox::CloudBox(glm::vec3 low, glm::vec3 high)
{
    boxLow = low;
    boxHigh = high;

    renderCloud = new Gloom::Shader();
    renderCloud->makeBasicShader("../res/shaders/cloud.vert", "../res/shaders/cloud.frag");
    renderCloud->activate();
    glUniform4fv(renderCloud->getUniformFromName("viewport"), 1, glm::value_ptr(glm::vec4(0.,0.,windowWidth, windowHeight)));
    glUniform2fv(renderCloud->getUniformFromName("boxHigh"), 1, glm::value_ptr(boxHigh));
    glUniform2fv(renderCloud->getUniformFromName("boxLow"), 1, glm::value_ptr(boxLow));
    glUniform2fv(renderCloud->getUniformFromName("iResolution"), 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));
    Mesh screen_quad = generateScreenQuad();
    vao = generateBuffer(screen_quad);
}

void CloudBox::render(Gloom::Camera *camera)
{
    renderCloud->activate();

    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
    glm::mat4 VP = projection * camera->getViewMatrix();
    glm::mat4 inverse_mvp = glm::inverse(VP);
    glm::vec3 camera_position = glm::vec3(camera->getViewMatrix()[3]);
    glUniform1f(renderCloud->getUniformFromName("iTime"), (float)glfwGetTime());
    glUniformMatrix4fv(renderCloud->getUniformFromName("inverse_vp"), 1, false, glm::value_ptr(inverse_mvp));
    glUniform3fv(renderCloud->getUniformFromName("eyePos"), 1, glm::value_ptr(camera_position));
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}