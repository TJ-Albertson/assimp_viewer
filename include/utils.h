#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    float nx, ny, nz;
} Normal;

typedef struct {
    int vertex_index, normal_index;
} FaceVertex;

typedef struct {
    FaceVertex vertices[3]; // Each face consists of three vertices with normals
} Face;


struct Polygon2 {
    glm::vec3* vertices;
    glm::vec3 normal;
};

int extract_faces_from_obj(const char* file_path)
{
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    float vertices[1000][3]; // Assuming no more than 1000 vertices
    float normals[1000][3]; // Assuming no more than 1000 normals
    int vertexCount = 0;
    int normalCount = 0;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v') {
            if (line[1] == ' ') {
                float x, y, z;
                sscanf(line, "v %f %f %f", &x, &y, &z);
                vertices[vertexCount][0] = x;
                vertices[vertexCount][1] = y;
                vertices[vertexCount][2] = z;
                vertexCount++;
            } else if (line[1] == 'n') {
                float nx, ny, nz;
                sscanf(line, "vn %f %f %f", &nx, &ny, &nz);
                normals[normalCount][0] = nx;
                normals[normalCount][1] = ny;
                normals[normalCount][2] = nz;
                normalCount++;
            }
        } else if (line[0] == 'f') {
            int v1, v2, v3, vn1, vn2, vn3;
            sscanf(line, "f %d//%d %d//%d %d//%d", &v1, &vn1, &v2, &vn2, &v3, &vn3);

            // Print face vertex and normal information
            printf("Face Vertex Indices: %d %d %d\n", v1, v2, v3);
            printf("Face Normal Indices: %d %d %d\n", vn1, vn2, vn3);
        }
    }

    fclose(file);
    return 0;
}

int print_faces(std::string path)
{
    const char* file_path = path.c_str();

    extract_faces_from_obj(file_path);
    return 0;
}
