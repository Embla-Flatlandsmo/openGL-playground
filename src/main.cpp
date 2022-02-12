// Local headers
#include "utilities/window.hpp"
#include "program.hpp"

#include "utilities/imgui/imgui.h"
#include "utilities/imgui/imgui_impl_glfw.h"
#include "utilities/imgui/imgui_impl_opengl3.h"

// System headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard headers
#include <cstdlib>
#include <arrrgh.hpp>


// A callback which allows GLFW to report errors whenever they occur
static void glfwErrorCallback(int error, const char *description)
{
    fprintf(stderr, "GLFW returned an error:\n\t%s (%i)\n", description, error);
}


GLFWwindow* initialise()
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Could not start GLFW\n");
        exit(EXIT_FAILURE);
    }

    // Set core window options (adjust version numbers if needed)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Enable the GLFW runtime error callback function defined previously.
    glfwSetErrorCallback(glfwErrorCallback);

    // Set additional window options
    glfwWindowHint(GLFW_RESIZABLE, windowResizable);
    glfwWindowHint(GLFW_SAMPLES, windowSamples);  // MSAA

    // Create window using GLFW
    GLFWwindow* window = glfwCreateWindow(windowWidth,
                                          windowHeight,
                                          windowTitle.c_str(),
                                          nullptr,
                                          nullptr);

    // Ensure the window is set up correctly
    if (!window)
    {
        fprintf(stderr, "Could not open GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Let the window be the current OpenGL context and initialise glad
    glfwMakeContextCurrent(window);
    gladLoadGL();

    // Print various OpenGL information to stdout
    printf("%s: %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    printf("GLFW\t %s\n", glfwGetVersionString());
    printf("OpenGL\t %s\n", glGetString(GL_VERSION));
    printf("GLSL\t %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    int32_t gsx, gsy, gsz, gcx, gcy, gcz, inv;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &gsx);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &gsy);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &gsz);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &gcx);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &gcy);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &gcz);
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &inv);

    printf("Max work group size %i %i %i\n", gsx, gsy, gsz);
    printf("Max work group count %i %i %i\n",gcx, gcy, gcz);
    printf("Max invocations      %i\n", inv);

    return window;
}


int main(int argc, const char* argb[])
{
    arrrgh::parser parser("glowbox", "Small breakout like juggling game");
    const auto& showHelp = parser.add<bool>("help", "Show this help message.", 'h', arrrgh::Optional, false);
    const auto& enableMusic = parser.add<bool>("enable-music", "Play background music while the game is playing", 'm', arrrgh::Optional, false);
    const auto& enableAutoplay = parser.add<bool>("autoplay", "Let the game play itself automatically. Useful for testing.", 'a', arrrgh::Optional, false);

    // If you want to add more program arguments, define them here,
    // but do not request their value here (they have not been parsed yet at this point).

    try
    {
        parser.parse(argc, argb);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error parsing arguments: " << e.what() << std::endl;
        parser.show_usage(std::cerr);
        exit(1);
    }

    // Show help if desired
    if(showHelp.value())
    {
        return 0;
    }

    CommandLineOptions options;
    options.enableMusic = enableMusic.value();
    options.enableAutoplay = enableAutoplay.value();

    // Initialise window using GLFW
    GLFWwindow* window = initialise();

    // UI panels initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.WantCaptureMouse = true;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");

    
    // Run an OpenGL application using this window
    runProgram(window, options);

    // Terminate GLFW (no need to call glfwDestroyWindow)

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    return EXIT_SUCCESS;
}
