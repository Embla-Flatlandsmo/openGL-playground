#include <iostream>
#include <fmt/format.h>
#include "shapes.h"

#define M_PI 3.14159265359f

Mesh cube(glm::vec3 scale, glm::vec2 textureScale, bool tilingTextures, bool inverted, glm::vec3 textureScale3d)
{
    glm::vec3 points[8];
    int indices[36];

    for (int y = 0; y <= 1; y++)
        for (int z = 0; z <= 1; z++)
            for (int x = 0; x <= 1; x++)
            {
                points[x + y * 4 + z * 2] = glm::vec3(x * 2 - 1, y * 2 - 1, z * 2 - 1) * 0.5f * scale;
            }

    int faces[6][4] = {
        {2, 3, 0, 1}, // Bottom
        {4, 5, 6, 7}, // Top
        {7, 5, 3, 1}, // Right
        {4, 6, 0, 2}, // Left
        {5, 4, 1, 0}, // Back
        {6, 7, 2, 3}, // Front
    };

    scale = scale * textureScale3d;
    glm::vec2 faceScale[6] = {
        {-scale.x, -scale.z}, // Bottom
        {-scale.x, -scale.z}, // Top
        {scale.z, scale.y},   // Right
        {scale.z, scale.y},   // Left
        {scale.x, scale.y},   // Back
        {scale.x, scale.y},   // Front
    };

    glm::vec3 normals[6] = {
        {0, -1, 0}, // Bottom
        {0, 1, 0},  // Top
        {1, 0, 0},  // Right
        {-1, 0, 0}, // Left
        {0, 0, -1}, // Back
        {0, 0, 1},  // Front
    };

    glm::vec2 UVs[4] = {
        {0, 0},
        {0, 1},
        {1, 0},
        {1, 1},
    };

    Mesh m;
    for (int face = 0; face < 6; face++)
    {
        int offset = face * 6;
        indices[offset + 0] = faces[face][0];
        indices[offset + 3] = faces[face][0];

        if (!inverted)
        {
            indices[offset + 1] = faces[face][3];
            indices[offset + 2] = faces[face][1];
            indices[offset + 4] = faces[face][2];
            indices[offset + 5] = faces[face][3];
        }
        else
        {
            indices[offset + 1] = faces[face][1];
            indices[offset + 2] = faces[face][3];
            indices[offset + 4] = faces[face][3];
            indices[offset + 5] = faces[face][2];
        }

        for (int i = 0; i < 6; i++)
        {
            m.vertices.push_back(points[indices[offset + i]]);
            m.indices.push_back(offset + i);
            m.normals.push_back(normals[face] * (inverted ? -1.f : 1.f));
        }

        glm::vec2 textureScaleFactor = tilingTextures ? (faceScale[face] / textureScale) : glm::vec2(1);

        if (!inverted)
        {
            for (int i : {1, 2, 3, 1, 0, 2})
            {
                m.textureCoordinates.push_back(UVs[i] * textureScaleFactor);
            }
        }
        else
        {
            for (int i : {3, 1, 0, 3, 0, 2})
            {
                m.textureCoordinates.push_back(UVs[i] * textureScaleFactor);
            }
        }
    }

    return m;
}

Mesh generateSphere(float sphereRadius, int slices, int layers)
{
    const unsigned int triangleCount = slices * layers * 2;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    std::vector<glm::vec2> uvs;

    vertices.reserve(3 * triangleCount);
    normals.reserve(3 * triangleCount);
    indices.reserve(3 * triangleCount);

    // Slices require us to define a full revolution worth of triangles.
    // Layers only requires angle varying between the bottom and the top (a layer only covers half a circle worth of angles)
    const float degreesPerLayer = 180.0 / (float)layers;
    const float degreesPerSlice = 360.0 / (float)slices;

    unsigned int i = 0;

    // Constructing the sphere one layer at a time
    for (int layer = 0; layer < layers; layer++)
    {
        int nextLayer = layer + 1;

        // Angles between the vector pointing to any point on a particular layer and the negative z-axis
        float currentAngleZDegrees = degreesPerLayer * layer;
        float nextAngleZDegrees = degreesPerLayer * nextLayer;

        // All coordinates within a single layer share z-coordinates.
        // So we can calculate those of the current and subsequent layer here.
        float currentZ = -cos(glm::radians(currentAngleZDegrees));
        float nextZ = -cos(glm::radians(nextAngleZDegrees));

        // The row of vertices forms a circle around the vertical diagonal (z-axis) of the sphere.
        // These radii are also constant for an entire layer, so we can precalculate them.
        float radius = sin(glm::radians(currentAngleZDegrees));
        float nextRadius = sin(glm::radians(nextAngleZDegrees));

        // Now we can move on to constructing individual slices within a layer
        for (int slice = 0; slice < slices; slice++)
        {

            // The direction of the start and the end of the slice in the xy-plane
            float currentSliceAngleDegrees = slice * degreesPerSlice;
            float nextSliceAngleDegrees = (slice + 1) * degreesPerSlice;

            // Determining the direction vector for both the start and end of the slice
            float currentDirectionX = cos(glm::radians(currentSliceAngleDegrees));
            float currentDirectionY = sin(glm::radians(currentSliceAngleDegrees));

            float nextDirectionX = cos(glm::radians(nextSliceAngleDegrees));
            float nextDirectionY = sin(glm::radians(nextSliceAngleDegrees));

            vertices.emplace_back(sphereRadius * radius * currentDirectionX,
                                  sphereRadius * radius * currentDirectionY,
                                  sphereRadius * currentZ);
            vertices.emplace_back(sphereRadius * radius * nextDirectionX,
                                  sphereRadius * radius * nextDirectionY,
                                  sphereRadius * currentZ);
            vertices.emplace_back(sphereRadius * nextRadius * nextDirectionX,
                                  sphereRadius * nextRadius * nextDirectionY,
                                  sphereRadius * nextZ);
            vertices.emplace_back(sphereRadius * radius * currentDirectionX,
                                  sphereRadius * radius * currentDirectionY,
                                  sphereRadius * currentZ);
            vertices.emplace_back(sphereRadius * nextRadius * nextDirectionX,
                                  sphereRadius * nextRadius * nextDirectionY,
                                  sphereRadius * nextZ);
            vertices.emplace_back(sphereRadius * nextRadius * currentDirectionX,
                                  sphereRadius * nextRadius * currentDirectionY,
                                  sphereRadius * nextZ);

            normals.emplace_back(radius * currentDirectionX,
                                 radius * currentDirectionY,
                                 currentZ);
            normals.emplace_back(radius * nextDirectionX,
                                 radius * nextDirectionY,
                                 currentZ);
            normals.emplace_back(nextRadius * nextDirectionX,
                                 nextRadius * nextDirectionY,
                                 nextZ);
            normals.emplace_back(radius * currentDirectionX,
                                 radius * currentDirectionY,
                                 currentZ);
            normals.emplace_back(nextRadius * nextDirectionX,
                                 nextRadius * nextDirectionY,
                                 nextZ);
            normals.emplace_back(nextRadius * currentDirectionX,
                                 nextRadius * currentDirectionY,
                                 nextZ);

            indices.emplace_back(i + 0);
            indices.emplace_back(i + 1);
            indices.emplace_back(i + 2);
            indices.emplace_back(i + 3);
            indices.emplace_back(i + 4);
            indices.emplace_back(i + 5);

            for (int j = 0; j < 6; j++)
            {
                glm::vec3 vertex = vertices.at(i + j);
                uvs.emplace_back(
                    0.5 + (glm::atan(vertex.z, vertex.y) / (2.0 * M_PI)),
                    0.5 - (glm::asin(vertex.y) / M_PI));
            }

            i += 6;
        }
    }

    Mesh mesh;
    mesh.vertices = vertices;
    mesh.normals = normals;
    mesh.indices = indices;
    mesh.textureCoordinates = uvs;
    return mesh;
}

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

    // glm::vec3 normals[6] = {
    //     {0, -1, 0}, // Bottom
    //     {0, 1, 0},  // Top
    //     {1, 0, 0},  // Right
    //     {-1, 0, 0}, // Left
    //     {0, 0, -1}, // Back
    //     {0, 0, 1},  // Front
    // };

    // glm::vec2 UVs[4] = {
    //     {0, 0},
    //     {0, 1},
    //     {1, 0},
    //     {1, 1},
    // };

    Mesh m;
    for (int i = 0; i < 8; i++) {
        m.vertices.push_back(points[i]);
    }
    for (int i = 0; i < 24; i++) {
        m.indices.push_back(indices[i]);
    }
    return m;
}