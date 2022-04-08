#include <iostream>
#include <fmt/format.h>
#include "shapes.h"

#define M_PI 3.14159265359f

Mesh generateBoundingBoxMesh(glm::vec3 scale)
{
    // glm::vec3 points[8];
    // int indices[36];
    glm::vec3 points[8] = {
        {0, 0, 0},
        {1, 0, 0},
        {1, 1, 0},
        {0, 1, 0},
        {0, 0, 1},
        {1, 0, 1},
        {1, 1, 1},
        {0, 1, 1}};

    uint32_t indices[24] = {
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

    Mesh m;
    for (int i = 0; i < 8; i++) {
        m.vertices.push_back(points[i]);
    }
    for (int i = 0; i < 24; i++) {
        m.indices.push_back(indices[i]);
    }
    return m;
}

Mesh generateScreenQuad(void) 
{
    
    std::vector<glm::vec3> vertices = {
        {-1.0f, -1.0f, 0.0f},
        {1.0f, -1.0f, 0.0f},
        {-1.0f,  1.0f, 0.0f},
        { 1.0f,  1.0f, 0.0f}
    };

    std::vector<glm::vec2> uvs = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {0.0f, 1.0f},
        {1.0f, 1.0f}
    };

    std::vector<unsigned int> indices = {
        0, 1, 2, // Triangle 1
        2, 1, 3  // Triangle 2
    };

    std::vector<glm::vec3> normals = {
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f},
    };

    Mesh mesh;
    mesh.vertices = vertices;
    mesh.normals = normals;
    mesh.indices = indices;
    mesh.textureCoordinates = uvs;
    return mesh;
}