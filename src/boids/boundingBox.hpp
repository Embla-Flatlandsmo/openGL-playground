#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <utilities/camera.hpp>
#include <utilities/mesh.h>
#include <utilities/shader.hpp>


/**
 * @brief Representation of bounding box
 * 
 * Bounding box defined by two diagonally opposite vertices
 */
class BoundingBox
{
public:
    /**
     * @brief Construct a new Bounding Box object
     * 
     * @param low Lowest values (xyz) of bounding box
     * @param high Highest values (xyz) of bounding box
     */
    BoundingBox(glm::vec3 low, glm::vec3 high);
    /**
     * @brief Destroy the Bounding Box object
     */
    ~BoundingBox();
    /**
     * @brief Render the bounding box as a wireframe
     * 
     * @param window window to render in
     * @param camera camera input
     */
    void renderAsWireframe(GLFWwindow *window, Gloom::Camera *camera);
    /**
     * @brief Minimum coordinates of bounding box
     */
    glm::vec3 low;
    /**
     * @brief Maximal coordinates of bounding box
     */
    glm::vec3 high;

private:
    /**
     * @brief Program with vertex and fragment shader to render the box
     */
    Gloom::Shader *boxProgram;

    Mesh box;

    /**
     * @brief VAO for bounding box
     */
    unsigned int boxVAO;
};
