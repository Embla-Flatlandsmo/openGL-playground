#include "boundingBox.hpp"
#include <utilities/shader.hpp>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

//TODO: find a better way to handle this plz
#define WIREFRAME_NUM_INDICES 48

unsigned int wireframeCube(glm::vec3 scale)
{
    // glm::vec3 points[8];
    // int indices[48];

    float positions[] = {
        0.0f, 0.0f, 1.0f, //0
        1.0f, 0.0f, 1.0f, //1
        1.0f, 1.0f, 1.0f, //2
        0.0f, 1.0f, 1.0f, //3
        0.0f, 1.0f, 0.0f, //4
        1.0f, 1.0f, 0.0f, //5
        0.0f, 0.0f, 0.0f, //6
        1.0f, 0.0f, 0.0f  //7
    };

    //Transform positions to scale:
    for (int i = 0; i < 24; i++) {
        if (i%3 == 0) {
            positions[i] *= scale.x;
        } else if (i % 3 == 1) {
            positions[i] *= scale.y;
        } else {
            positions[i] *= scale.z;
        }
    }

    unsigned int indices[] = {
                                0, 1, 1, 2, 2, 3, 3, 0,     // fronte
                                0, 6, 6, 7, 7, 1, 1, 0,     // sotto
                                3, 2, 2, 5, 5, 4, 4, 3,     // sopra
                                4, 5, 5, 7, 7, 6, 6, 4,     // retro
                                2, 5, 5, 7, 7, 1, 1, 2,     // destra
                                3, 4, 4, 6, 6, 0, 0, 3      // sinistra

    };

    unsigned int vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    unsigned int bufferID;
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(float), positions, GL_STATIC_DRAW);
    
    unsigned int indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, WIREFRAME_NUM_INDICES*sizeof(unsigned int), indices, GL_STATIC_DRAW);

    return vaoID;
}

BoundingBox::BoundingBox(glm::vec3 low, glm::vec3 high)
{
    this->low = low;
    this->high = high;
    boxProgram = new Gloom::Shader();

    boxProgram->makeBasicShader("res/shaders/box.vert", "res/shaders/box.frag");
    // boxProgram->activate();
    glm::vec3 diagonal = glm::vec3(high.x-low.x, high.y-low.y, high.z-low.z);
    box = boundingBoxMesh(diagonal);
    boxVAO  = generateBuffer(box);

    // boxVAO = wireframeCube(high-low);
    // boxProgram->deactivate();
}

BoundingBox::~BoundingBox()
{
    boxProgram->destroy();
}

void BoundingBox::renderAsWireframe(GLFWwindow *window, Gloom::Camera *camera)
{
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glm::mat4 model = glm::translate(low);
    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
    glm::mat4 MVP = projection * camera->getViewMatrix()*model;

    boxProgram->activate();
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
    glLineWidth(3);
    // glEnableVertexAttribArray(0);
    glBindVertexArray(boxVAO);
    glUniformMatrix4fv(boxProgram->getUniformFromName("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
    // glDrawElements(GL_LINE_LOOP, WIREFRAME_NUM_INDICES, GL_UNSIGNED_INT, nullptr);
    glDrawElements(GL_LINES, box.indices.size(), GL_UNSIGNED_INT, nullptr);
    boxProgram->deactivate();
}

