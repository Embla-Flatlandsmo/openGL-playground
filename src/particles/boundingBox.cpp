// Local headers
#include "boundingBox.hpp"
#include <utilities/shader.hpp>

// System headers
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <utilities/shapes.h>
#include <utilities/glutils.h>
#define WIREFRAME_NUM_INDICES 24


/**
 * @brief Helper function for creating the 
 * vertices and indices to draw the bounding box
 * 
 * @param scale How large the cube is (x, y, z)
 * @return VAO identifier
 */
unsigned int createWireframeCube(glm::vec3 scale)
{
    /**
     * @brief Coordinates for the cube
     */
    glm::vec3 points[8] = {
        {0, 0, 0},
        {1, 0, 0},
        {1, 1, 0},
        {0, 1, 0},
        {0, 0, 1},
        {1, 0, 1},
        {1, 1, 1},
        {0, 1, 1}};

    /**
     * @brief Indices describing how the coordinates
     * fit together
     */
    uint32_t indices[WIREFRAME_NUM_INDICES] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        0, 4,
        1, 5,
        2, 6,
        3, 7,
        4, 5,
        5, 6,
        6, 7,
        7, 4};
    
    for (int i = 0; i < 8; i++) {
        points[i] *= scale;
    }
    unsigned int vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    // Create a VBO for the points
    unsigned int bufferID;
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ARRAY_BUFFER, 8*sizeof(glm::vec3), points, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    glEnableVertexAttribArray(0);

    // Create the index buffer for the points
    unsigned int indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, WIREFRAME_NUM_INDICES * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    return vaoID;

}

BoundingBox::BoundingBox(glm::vec3 low, glm::vec3 high, float cell_size)
{
    this->low = low;
    this->high = high;
    // glm::uvec3 res = glm::round((high-low)/cell_size);

    // this->resolution = glm::floor((high-low)/cell_size);
    this->resolution = glm::uvec3(16, 16, 8);
    printf("GridRes: [%d, %d, %d]\n", resolution.x, resolution.y, resolution.z);
    numCells = this->resolution.x*this->resolution.y*this->resolution.z;
    boxShader = new Gloom::Shader();
    boxShader->makeBasicShader("../res/shaders/box.vert", "../res/shaders/box.frag");
    Mesh box = generateBoundingBoxMesh(high-low);
    boxVAO = generateBuffer(box);
}

BoundingBox::~BoundingBox()
{
    // TODO
    boxShader->destroy();
}

void BoundingBox::renderAsWireframe(GLFWwindow *window, Gloom::Camera *camera)
{
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glm::mat4 model = glm::translate(low);
    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
    glm::mat4 MVP = projection * camera->getViewMatrix()*model;

    // Use the box shader to render the bounding box
    boxShader->activate();
    glLineWidth(3);
    glBindVertexArray(boxVAO);
    glUniformMatrix4fv(boxShader->getUniformFromName("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
    glDrawElements(GL_LINES, WIREFRAME_NUM_INDICES, GL_UNSIGNED_INT, nullptr);
    boxShader->deactivate();
}

