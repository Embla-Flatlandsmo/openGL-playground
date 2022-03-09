// Local headers
#include "cloudBox.hpp"
#include <utilities/shader.hpp>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <utilities/window.hpp>
#include <utilities/camera.hpp>
#include <utilities/texture.hpp>

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

    renderCloud = new Gloom::Shader();
    renderCloud->makeBasicShader("../res/shaders/cloud.vert", "../res/shaders/cloud.frag");
    // renderCloud->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    
    renderCloud->activate();
    glUniform4fv(renderCloud->getUniformFromName("viewport"), 1, glm::value_ptr(glm::vec4(0.,0.,windowWidth, windowHeight)));
    glUniform2fv(renderCloud->getUniformFromName("boxHigh"), 1, glm::value_ptr(boxHigh));
    glUniform2fv(renderCloud->getUniformFromName("boxLow"), 1, glm::value_ptr(boxLow));
    glUniform2fv(renderCloud->getUniformFromName("iResolution"), 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));
    Mesh box = cube(glm::vec3(20,20,20), glm::vec2(10), false, false);
    vao = generateBuffer(box);
    
    // Mesh screen_quad = generateScreenQuad();
    // vao = generateBuffer(screen_quad);

    generateTextures();
}

void CloudBox::render(Gloom::Camera *camera)
{
    renderCloud->activate();

    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
    glm::mat4 VP = projection * camera->getViewMatrix();
    glm::mat4 inverse_mvp = glm::inverse(VP);
    glm::vec3 camera_position = glm::vec3(camera->getViewMatrix()[3]);


    glUniformMatrix4fv(renderCloud->getUniformFromName("MVP"), 1, GL_FALSE, glm::value_ptr(VP));
    glUniform1f(renderCloud->getUniformFromName("iTime"), (float)glfwGetTime());
    glUniformMatrix4fv(renderCloud->getUniformFromName("inv_vp"), 1, false, glm::value_ptr(inverse_mvp));
    glUniformMatrix4fv(renderCloud->getUniformFromName("inv_view"), 1, false, glm::value_ptr(glm::inverse(camera->getViewMatrix())));
    glUniformMatrix4fv(renderCloud->getUniformFromName("inv_proj"), 1, false, glm::value_ptr(glm::inverse(projection)));
    glUniform3fv(renderCloud->getUniformFromName("cam_pos"), 1, glm::value_ptr(camera_position));
    glUniform4fv(renderCloud->getUniformFromName("viewport"), 1, glm::value_ptr(glm::vec4(0.0,0.0, windowWidth, windowHeight)));

    glActiveTexture(GL_TEXTURE0);
    glBindTextureUnit(GL_TEXTURE_3D, perlinTex);

    glActiveTexture(GL_TEXTURE1);
    glBindTextureUnit(GL_TEXTURE_3D, worley32);    
    
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
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
    perlinWorley->activate();
    glUniform3fv(perlinWorley->getUniformFromName("u_resolution"), 1, glm::value_ptr(glm::vec3(128, 128, 128)));
    std::cout << "computing perlinworley!" << std::endl;
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(perlinWorley->getUniformFromName("outVolTex"), 0);
    glBindTexture(GL_TEXTURE_3D, this->perlinTex);
    glBindImageTexture(0, this->perlinTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glDispatchCompute(INT_CEIL(128, 4), INT_CEIL(128, 4), INT_CEIL(128, 4));
    std::cout << "computed!!" << std::endl;
    //glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glGenerateMipmap(GL_TEXTURE_3D);

    //compute shaders
    worley = new Gloom::Shader();
    worley->attach("../res/shaders/cloud/worley.comp");
    worley->link();

    //make texture
    this->worley32 = generateTexture3D(32, 32, 32);

    //compute
    worley->activate();

    //worley_git.setVec3("u_resolution", glm::vec3(32, 32, 32));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, this->worley32);
    glBindImageTexture(0, this->worley32, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    std::cout << "computing worley 32!" << std::endl;
    glDispatchCompute(INT_CEIL(32, 4), INT_CEIL(32, 4), INT_CEIL(32, 4));
    std::cout << "computed!!" << std::endl;
    glGenerateMipmap(GL_TEXTURE_3D);


	////////////////////////

	// if (!weatherTex) {
	// 	//make texture
	// 	this->weatherTex = generateTexture2D(1024, 1024);

	// 	//compute
	// 	generateWeatherMap();

	// 	seed = scene->seed;
	// 	oldSeed = seed;
	// }
}