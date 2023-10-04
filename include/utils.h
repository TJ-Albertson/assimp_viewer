#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// squares only
struct Poly {
    glm::vec3 vertices[4];
    glm::vec3 normal;
};

std::string filepath(std::string path)
{
    std::filesystem::path currentDir = std::filesystem::current_path();

    std::string projectPath = currentDir.string();

    for (char& c : projectPath) {
        if (c == '\\') {
            c = '/';
        }
    }

    std::string toRemove = "/assimp_viewer";

    size_t pos = projectPath.rfind(toRemove);
    if (pos != std::string::npos) {
        projectPath.erase(pos, toRemove.length());
    }

    return projectPath.append(path);
}

int extract_faces_from_obj(const char* file_path)
{
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    int vertexCount = 0;
    int normalCount = 0;
    int faceCount = 0;
    
    glm::vec3 vertices[1000];
    glm::vec3 normals[1000];

    Poly faces[6];

    char line[256];

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v') {
            if (line[1] == ' ') {
                float x, y, z;
                sscanf(line, "v %f %f %f", &x, &y, &z);
                vertices[vertexCount] = glm::vec3(x, y, z);
                vertexCount++;
            } else if (line[1] == 'n') {
                float nx, ny, nz;
                sscanf(line, "vn %f %f %f", &nx, &ny, &nz);
                normals[normalCount] = glm::vec3(nx, ny, nz);
                normalCount++;
            }
        } else if (line[0] == 'f') {
            int point1, point2, point3, point4;
            sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &point1, &point2, &point3, &point4);

            faces[faceCount].vertices[0] = vertices[point1 - 1];
            faces[faceCount].vertices[1] = vertices[point2 - 1];
            faces[faceCount].vertices[2] = vertices[point3 - 1];
            faces[faceCount].vertices[3] = vertices[point4 - 1];
            
            faces[faceCount].normal = normals[faceCount];

            faceCount++;
        }
    }

    fclose(file);

    for (int i = 0; i < faceCount; ++i) {
        printf("Face: %d\n", i);
        printf("    Vertices: ");

        for (int j = 0; j < 4; ++j) {
            glm::vec3 vertex = faces[i].vertices[j];
            printf("{%.2f,%.2f,%.2f} ", vertex.x, vertex.y, vertex.z);
        }
        printf("\n    Normal: %.2f %.2f %.2f\n", normals[i].x, normals[i].y, normals[i].z);
    }
    printf("\n");

    return 0;
}

int print_faces(std::string path)
{
    const char* file_path = path.c_str();

    extract_faces_from_obj(file_path);
    return 0;
}