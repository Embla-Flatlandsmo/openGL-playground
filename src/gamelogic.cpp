#include <chrono>

#include "utilities/imgui/imgui.h"
#include "utilities/imgui/imgui_impl_glfw.h"
#include "utilities/imgui/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>


#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <utilities/camera.hpp>
#include <utilities/objLoader.hpp>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"
#include "particles/particleSystem.hpp"
#include "clouds/cloudBox.hpp"
#include "sky/sky.hpp"

enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

double padPositionX = 0;
double padPositionZ = 0;

unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

SceneNode* rootNode;
SceneNode* boxNode;
SceneNode* ballNode;
SceneNode* padNode;

// SceneNode* cloudNode;


double ballRadius = 3.0f;

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
sf::Sound* sound;
Gloom::Camera* camera;
ParticleSystem* particles;
CloudBox* cloud;
Sky* sky;

ScreenQuad* screen;

// const glm::vec3 boxDimensions(180, 90, 90);
const glm::vec3 boxDimensions(300, 150, 150);
const glm::vec3 padDimensions(30, 3, 40);

glm::vec3 ballPosition(0, ballRadius + padDimensions.y, boxDimensions.z / 2);
glm::vec3 ballDirection(1, 1, 0.2f);

CommandLineOptions options;

bool hasStarted = false;
bool hasLost = false;
bool jumpedToNextFrame = false;
bool isPaused = false;

bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime = debug_startTime;

double mouseSensitivity = 1.0;
double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;


void mouseCallback(GLFWwindow* window, double x, double y) {
    camera->handleCursorPosInput(x, y);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    camera->handleMouseButtonInputs(button, action);

}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

    camera->handleKeyboardInputs(key, action);
    if (action == GLFW_PRESS) {
        switch(key) 
        {
            case GLFW_KEY_U:
                screen->incrementCurrentEffect();
                break;
            case GLFW_KEY_J:
                screen->decrementCurrentEffect();
                break;
        }
    }
}

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
        return;
    }

    options = gameOptions;

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    // Callbacks setup for user input
    // glfwSetCursorPosCallback(window, mouseCallback);
    // glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyboardCallback);

    screen = new ScreenQuad();
    camera = new Gloom::Camera(glm::vec3(20, 20, 70));
    shader = new Gloom::Shader();
    sky = new Sky();
    shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    shader->activate();


    // Create meshes
    Mesh box = cube(boxDimensions, glm::vec2(90), true, true);

    // Fill buffers
    unsigned int boxVAO  = generateBuffer(box);
    // Construct scene
    rootNode = createSceneNode();
    boxNode  = createSceneNode();

    rootNode->children.push_back(boxNode);
    boxNode->vertexArrayObjectID = boxVAO;
    boxNode->VAOIndexCount = box.indices.size();
    cloud = new CloudBox(glm::vec3(-150.,-20.,-150.), glm::vec3(150., 50., 150.)); // Why is it multiplied by 10?
    particles = new ParticleSystem(glm::vec3(10.,10.,10.), glm::vec3(40., 40., 40.));

    getTimeDeltaSeconds();

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateFrame(GLFWwindow* window) {
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double timeDelta = getTimeDeltaSeconds();

    camera->updateCamera(timeDelta);
    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);

    glm::mat4 VP = projection * camera->getViewMatrix();
    particles->update();
    updateNodeTransformations(rootNode, VP);
}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar) {
    glm::mat4 transformationMatrix =
              glm::translate(node->position)
            * glm::translate(node->referencePoint)
            * glm::rotate(node->rotation.y, glm::vec3(0,1,0))
            * glm::rotate(node->rotation.x, glm::vec3(1,0,0))
            * glm::rotate(node->rotation.z, glm::vec3(0,0,1))
            * glm::scale(node->scale)
            * glm::translate(-node->referencePoint);

    node->currentTransformationMatrix = transformationThusFar * transformationMatrix;

    switch(node->nodeType) {
        case GEOMETRY: break;
        case POINT_LIGHT: break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentTransformationMatrix);
    }
}

void renderNode(SceneNode* node) {
    glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        case POINT_LIGHT: break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) { 
        renderNode(child);
    }
}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    screen->bindFramebuffer();
    sky->render(camera);
    particles->render(window, camera);

    // shader->activate();
    // renderNode(rootNode);

    screen->unbindFramebuffer();


    screen->draw();
    cloud->setDepthBuffer(screen->depth_texture);
    cloud->setColorBuffer(screen->color_texture);
    cloud->render(camera);
}

void renderUI(void) {
    // ImGui::ShowDemoWindow
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    ImGui::Begin("Boid properties");
    ImGui::SliderFloat("Size", &(particles->boidProperties.size), 0.1f, 2.0f);
    ImGui::SliderFloat("Cohesion", &(particles->boidProperties.cohesion_factor), 0.0f, 1.5f);
    ImGui::SliderFloat("Alignment", &(particles->boidProperties.alignment_factor), 0.0f, 1.5f);
    ImGui::SliderFloat("Separation", &(particles->boidProperties.separation_factor), 0.0f, 1.5f);
    ImGui::SliderFloat("Separation Range", &(particles->boidProperties.separation_range), 0.0f, 3.0f); // Max is view range
    ImGui::SliderFloat("Boundary avoidance", &(particles->boidProperties.boundary_avoidance_factor), 0.0f, 0.2f);
    ImGui::SliderFloat("dt", &(particles->boidProperties.dt), 0.0f, 2.0);
    ImGui::SliderFloat("Max velocity", &(particles->boidProperties.max_vel), 0.0f, 4.0f);
    ImGui::Checkbox("Wrap around", &(particles->boidProperties.wrap_around));
    ImGui::End();
    
    ImGui::Begin("Diagonstics");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
    ImGui::End();

    cloud->renderUI();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}