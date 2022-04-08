// Local headers
#include "cloudBox.hpp"
#include <utilities/shader.hpp>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <utilities/window.hpp>
#include <utilities/camera.hpp>
#include <utilities/texture.hpp>


#include "utilities/imgui/imgui.h"
#include "utilities/imgui/imgui_impl_glfw.h"
#include "utilities/imgui/imgui_impl_opengl3.h"

// System headers
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#define INT_CEIL(n,d) (int)ceil((float)n/d)

CloudBox::CloudBox(glm::vec3 low, glm::vec3 high)
{
    boxLow = low;
    boxHigh = high;
    rayMarchCloud = new Gloom::Shader();
    rayMarchCloud->attach("../res/shaders/cloud/ray_march.comp");
    rayMarchCloud->link();
    // screen = new ScreenQuad();
    renderCloud = new Gloom::Shader();
    // renderCloud->makeBasicShader("../res/shaders/cloud.vert", "../res/shaders/cloud.frag");
    // // renderCloud->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    
    // renderCloud->activate();
    // glUniform4fv(renderCloud->getUniformFromName("viewport"), 1, glm::value_ptr(glm::vec4(0.,0.,windowWidth, windowHeight)));
    // glUniform2fv(renderCloud->getUniformFromName("boxHigh"), 1, glm::value_ptr(boxHigh));
    // glUniform2fv(renderCloud->getUniformFromName("boxLow"), 1, glm::value_ptr(boxLow));
    // glUniform2fv(renderCloud->getUniformFromName("iResolution"), 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));
    // Mesh box = cube(glm::vec3(20,20,20), glm::vec2(10), false, false);
    // vao = generateBuffer(box);
    
    // Mesh screen_quad = generateScreenQuad();
    // vao = generateBuffer(screen_quad);

    generateTextures();
}

void CloudBox::setDepthBuffer(GLuint textureID)
{
    rayMarchCloud->activate();
    glActiveTexture(GL_TEXTURE0+2);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(rayMarchCloud->getUniformFromName("depth"), 2);
    rayMarchCloud->deactivate();
}

void CloudBox::setColorBuffer(GLuint textureID)
{
    rayMarchCloud->activate();
    glActiveTexture(GL_TEXTURE0+3);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(rayMarchCloud->getUniformFromName("fragColor"), 3);
    rayMarchCloud->deactivate();
}

void CloudBox::update(Gloom::Camera *camera)
{
    rayMarchCloud->activate();
    glm::mat4 projection = camera->getProjMatrix();
    glm::mat4 VP = projection * camera->getViewMatrix();
    glm::mat4 inverse_mvp = glm::inverse(VP);
    glUniform3fv(rayMarchCloud->getUniformFromName("AABBmin"), 1, glm::value_ptr(boxLow));
    glUniform3fv(rayMarchCloud->getUniformFromName("AABBmax"), 1, glm::value_ptr(boxHigh));
    glUniform3fv(rayMarchCloud->getUniformFromName("VolumeGridSize"), 1, glm::value_ptr(glm::vec3(128,128,128))); // Input to generateTexture3D in init

    glUniform1f(rayMarchCloud->getUniformFromName("time"), glfwGetTime());
    glUniformMatrix4fv(rayMarchCloud->getUniformFromName("vp"), 1, false, glm::value_ptr(VP));
    glUniformMatrix4fv(rayMarchCloud->getUniformFromName("inv_vp"), 1, false, glm::value_ptr(inverse_mvp));
    glUniformMatrix4fv(rayMarchCloud->getUniformFromName("inv_view"), 1, false, glm::value_ptr(glm::inverse(camera->getViewMatrix())));
    glUniformMatrix4fv(rayMarchCloud->getUniformFromName("inv_proj"), 1, false, glm::value_ptr(glm::inverse(projection)));
    glUniformMatrix4fv(rayMarchCloud->getUniformFromName("proj"), 1, false, glm::value_ptr(projection));
    glUniformMatrix4fv(rayMarchCloud->getUniformFromName("view"), 1, false, glm::value_ptr(camera->getViewMatrix()));
    glUniform4fv(rayMarchCloud->getUniformFromName("viewport"), 1, glm::value_ptr(glm::vec4(0.0,0.0, windowWidth, windowHeight)));

    glUniform1f(rayMarchCloud->getUniformFromName("StepSize"), cloud_properties.step_size);
    glUniform1f(rayMarchCloud->getUniformFromName("coverage_multiplier"), cloud_properties.coverage_multiplier);
    glUniform1f(rayMarchCloud->getUniformFromName("cloud_speed"), cloud_properties.cloud_speed);
    glUniform1f(rayMarchCloud->getUniformFromName("density_factor"), cloud_properties.density_factor);
    glUniform1f(rayMarchCloud->getUniformFromName("texture_scale"), cloud_properties.texture_scale);
    glUniform1f(rayMarchCloud->getUniformFromName("weather_texture_scale"), cloud_properties.weather_texture_scale);
    glUniform1f(rayMarchCloud->getUniformFromName("sun_power"), cloud_properties.sun_power);
    glUniform1f(rayMarchCloud->getUniformFromName("fog_factor"), cloud_properties.fog_factor);
    glUniform3fv(rayMarchCloud->getUniformFromName("light_direction"), 1, glm::value_ptr(glm::normalize(glm::vec3(0.5, 1.0, 0.5))));
    glUniform3fv(rayMarchCloud->getUniformFromName("cloud_shadow_color"), 1, glm::value_ptr(cloud_properties.cloud_shadow_color));
    glUniform3fv(rayMarchCloud->getUniformFromName("cloud_light_color"), 1, glm::value_ptr(cloud_properties.cloud_light_color));
    rayMarchCloud->deactivate();
}

void CloudBox::render(void)
{
    // glm::vec3 camera_position = glm::vec3(camera->getViewMatrix()[3]);
    
    rayMarchCloud->activate();

    // Set sampler 0
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_3D, perlinTex);
    glUniform1i(rayMarchCloud->getUniformFromName("perlinWorley"), 0);

    // Set sampler 1
    glActiveTexture(GL_TEXTURE0+1);
    glBindTexture(GL_TEXTURE_2D, weatherTex);
    glUniform1i(rayMarchCloud->getUniformFromName("weather"), 1);

    glBindImageTexture(0, screen.color_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glDispatchCompute(INT_CEIL(windowWidth,8), INT_CEIL(windowHeight,8), 1);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_3D, 0);
    rayMarchCloud->deactivate();
    screen.render();
}

void CloudBox::generateTextures()
{
	/////////////////// TEXTURE GENERATION //////////////////
    //compute shaders
    perlinWorley = new Gloom::Shader();
    perlinWorley->attach("../res/shaders/cloud/perlinworley.comp");
    perlinWorley->link();

    //make texture
    this->perlinTex = generateTexture3D(128, 128, 128);
    //compute
/*     perlinWorley->activate();
    glUniform3fv(perlinWorley->getUniformFromName("u_resolution"), 1, glm::value_ptr(glm::vec3(128, 128, 128)));
    std::cout << "computing perlinworley!" << std::endl;
    glUniform1i(perlinWorley->getUniformFromName("outVolTex"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, this->perlinTex);
    glBindImageTexture(0, this->perlinTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glDispatchCompute(INT_CEIL(128, 4), INT_CEIL(128, 4), INT_CEIL(128, 4));
    std::cout << "computed!!" << std::endl;
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glGenerateMipmap(GL_TEXTURE_3D);
 */
    perlinWorley->activate();
    glUniform1i(perlinWorley->getUniformFromName("noiseTex"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, this->perlinTex);
    glBindImageTexture(0, this->perlinTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glDispatchCompute(INT_CEIL(128, 4), INT_CEIL(128, 4), INT_CEIL(128, 4));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glGenerateMipmap(GL_TEXTURE_3D);

    weather = new Gloom::Shader();
    weather->attach("../res/shaders/cloud/weather.comp");
    weather->link();
    weatherTex = generateTexture2D(1024, 1024);
	weather->activate();
	// weather->setVec3("seed", 0);
	// weather->setFloat("perlinFrequency", 2.0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->weatherTex);
    // glBindImageTexture(0, weatherTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    // glUniform1i(rayMarchCloud->getUniformFromName("weather"), 1);
	glDispatchCompute(INT_CEIL(1024, 8), INT_CEIL(1024, 8), 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void CloudBox::setDebug(bool enable)
{
    debug = enable;
}

void CloudBox::renderUI()
{
    if (debug)
    {
        ImGui::Begin("Clouds properties");
        ImGui::SliderFloat("Step size", &cloud_properties.step_size, 0.5f, 3.f);
        ImGui::SliderFloat("Coverage", &cloud_properties.coverage_multiplier, 0.0f, 1.0f);
        ImGui::SliderFloat("Cloud speed", &cloud_properties.cloud_speed, 0.0f, 50.0f);
        ImGui::SliderFloat("Density Factor", &cloud_properties.density_factor, 0.01f, 1.0f);
        ImGui::SliderFloat("Texture Scale", &cloud_properties.texture_scale, 0.05f, 10.0f);
        ImGui::SliderFloat("Weather Texture Scale", &cloud_properties.weather_texture_scale, 0.05f, 10.0f);
        ImGui::SliderFloat("Sun power", &cloud_properties.sun_power, 0.0f, 200.0f);
        ImGui::SliderFloat("Fog factor", &cloud_properties.fog_factor, 0.0f, 3.f);
        ImGui::SliderFloat3("cloud Light color", (float*)&cloud_properties.cloud_light_color, 0.0f, 1.0f);
        ImGui::SliderFloat3("Cloud Shadow Color", (float*)&cloud_properties.cloud_shadow_color, 0.0f, 1.0f);
        ImGui::End();
    }
}

void CloudBox::updateSun(glm::vec3 direction)
{
    rayMarchCloud->activate();
    glUniform3fv(rayMarchCloud->getUniformFromName("light_direction"), 1, glm::value_ptr(direction));
    rayMarchCloud->deactivate();
}