#include <chrono>

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <iostream>

// ImGui
#include "utilities/imgui/imgui.h"
#include "utilities/imgui/imgui_impl_glfw.h"
#include "utilities/imgui/imgui_impl_opengl3.h"

// Various utils
#include "utilities/timeutils.h"
#include "utilities/camera.hpp"


// Boids, clouds and sky
#include "particles/particleSystem.hpp"
#include "clouds/cloudBox.hpp"
#include "sky/sky.hpp"


#define UNUSED(expr) (void)(expr) // Macro to silence wunused-parameter
bool toggleDebug = false;

const float move_speed_debug = 15.0f;
const float move_speed_normal = 3.0f;


// These are heap allocated, because they should not be initialised at the start of the program
Gloom::Camera* camera;
ParticleSystem* particles;
CloudBox* cloud;
Sky* sky;

ScreenQuad* screen;

void mouseCallback(GLFWwindow* window, double x, double y) {
    UNUSED(window);
    camera->handleCursorPosInput(x, y);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    UNUSED(window);
    UNUSED(mods);
    camera->handleMouseButtonInputs(button, action);

}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    UNUSED(window);
    UNUSED(scancode);
    UNUSED(mods);
    camera->handleKeyboardInputs(key, action);
    if (action == GLFW_PRESS) {
        switch(key) 
        {
            case GLFW_KEY_R:
                particles->resetPositions();
                break;
            case GLFW_KEY_Y:
                toggleDebug = !toggleDebug;
                particles->setDebug(toggleDebug);
                cloud->setDebug(toggleDebug);
                break;

        }
    }
}

/**
 * @brief Initializes everything needed for the interactive experience :)
 */
void initGame(GLFWwindow* window) {

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    // Callbacks setup for user input
    // glfwSetCursorPosCallback(window, mouseCallback);
    // glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyboardCallback);

    screen = new ScreenQuad();
    camera = new Gloom::Camera(glm::vec3(20, 20, 20));
    camera->setMoveSpeed(move_speed_normal);
    sky = new Sky();

    cloud = new CloudBox(glm::vec3(-150.,-20.,-150.), glm::vec3(150., 50., 150.)); // Why is it multiplied by 10?
    particles = new ParticleSystem(glm::vec3(10.,10.,10.), glm::vec3(40., 40., 40.));

    getTimeDeltaSeconds();

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateFrame(GLFWwindow* window) {
    UNUSED(window);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double timeDelta = getTimeDeltaSeconds();

    camera->updateCamera(timeDelta);
    sky->update(camera);
    cloud->update(camera);
    particles->update();
}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    screen->bindFramebuffer();
    sky->render();
    particles->render(window, camera);

    cloud->setDepthBuffer(screen->depth_texture);
    cloud->setColorBuffer(screen->color_texture);
    cloud->render();
    screen->unbindFramebuffer();

    screen->render();

}

void renderUI(void) {

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ImGui::ShowDemoWindow();

    const float DISTANCE = 10.0f;
    ImVec2 diagnostics_window_pos = ImVec2(DISTANCE, DISTANCE);
    ImVec2 diagnostics_window_pivot = ImVec2(0.0f, 0.0f);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowPos(diagnostics_window_pos, ImGuiCond_Always, diagnostics_window_pivot);
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    
    ImGui::Begin("Diagonstics", NULL, window_flags);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Text("Controls:\nCamera: WASDEQ, Arrows, N\nDebug: Y\nReset: R");
    ImGui::End();

    if (toggleDebug)
    {
        camera->setMoveSpeed(move_speed_debug);
        particles->renderUI();
        cloud->renderUI();
        screen->renderUI();
    } else 
    {
        camera->setMoveSpeed(move_speed_normal);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}