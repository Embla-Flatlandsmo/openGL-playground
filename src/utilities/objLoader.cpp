#include <utilities/objLoader.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <utilities/mesh.h>
#include <fstream>

/**
 * @brief Loads .OBJ files. From http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
 * 
 * @param filename .obj file to be loaded
 * @return Mesh containing the obj information
 */
Mesh loadObj(std::string const &filename)
{
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    // Load object file from source

    FILE* file = fopen(filename.c_str(), "r");
    if (file == NULL){
        fprintf(stderr,
                "Something went wrong when loading the obj file at \"%s\".\n"
                "The file may not exist or is currently inaccessible.\n",
                filename.c_str());
        return Mesh();
    }

    while (1) {
        char lineHeader[128];

        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF) break;
        if (strcmp(lineHeader, "v")==0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        } else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            temp_uvs.push_back(uv);
        } else if (strcmp(lineHeader, "vn")==0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        } else if (strcmp(lineHeader, "f")==0) {
            // std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file,"%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches != 9){
                printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                return Mesh();
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices    .push_back(uvIndex[0]);
            uvIndices    .push_back(uvIndex[1]);
            uvIndices    .push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }

    // Data is extracted, now we must format it in a way that openGL likes
    std::vector<glm::vec3> out_vertices;
    std::vector<glm::vec2> out_uvs;
    std::vector<glm::vec3> out_normals;
    std::vector<unsigned int> out_vertexIndices;

    for (int i = 0; i < vertexIndices.size(); i++) {
        unsigned int vertexIndex = vertexIndices[i];
        glm::vec3 vertex = temp_vertices[vertexIndex-1];
        out_vertices.push_back(vertex);
        
        unsigned int uvIndex = uvIndices[i];
        glm::vec2 uv = temp_uvs[uvIndex-1];
        out_uvs.push_back(uv);
    
        unsigned int normalIndex = normalIndices[i];
        glm::vec3 normal = temp_normals[normalIndex-1];
        out_normals.push_back(normal);

        out_vertexIndices.push_back(i);
    }
    printf("Obj file loaded with out_vertex=%d", out_vertices.size());
    // Print various OpenGL information to stdout
    // printf("%s: %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    // printf("GLFW\t %s\n", glfwGetVersionString());
    // printf("OpenGL\t %s\n", glGetString(GL_VERSION));
    // printf("GLSL\t %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    Mesh m;
    m.vertices = out_vertices;
    m.normals = out_normals;
    m.textureCoordinates = out_uvs;
    m.indices = out_vertexIndices;
    return m;
}


