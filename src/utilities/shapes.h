#pragma once
#include "mesh.h"

Mesh cube(glm::vec3 scale = glm::vec3(1), glm::vec2 textureScale = glm::vec2(1), bool tilingTextures = false, bool inverted = false, glm::vec3 textureScale3d = glm::vec3(1));
Mesh generateBox(float width, float height, float depth, bool flipFaces = false);
Mesh generateSphere(float radius, int slices, int layers);

Mesh generateBoundingBoxMesh(glm::vec3 scale);
Mesh generateBoid(float size);
Mesh generateScreenQuad(void);