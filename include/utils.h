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

    //float vertices[1000][3]; // Assuming no more than 1000 vertices
    //float normals[1000][3]; // Assuming no more than 1000 normals
    int vertexCount = 0;
    int normalCount = 0;
    int faceCount = 0;
    
    glm::vec3 vertices[1000];
    glm::vec3 normals[1000];

    // squares only
    struct Face {
        glm::vec3 vertices[4];
        glm::vec3 normal;
    };

    Face faces[6];

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

            printf("Face: %d\n", faceCount);
            printf("    Vertices: %d %d %d %d\n", point1, point2, point3, point4);
            printf("    Nomal: %f %f %f\n", normals[faceCount].x, normals[faceCount].y, normals[faceCount].z);

            faceCount++;
        }
    }

    fclose(file);

    /*
    for (int i; i < faceCount; ++i) {
        printf("Face %d:\n", i);
        printf("    Vertices: %asdfsdf\n")
    }
    */


    return 0;
}

int print_faces(std::string path)
{
    const char* file_path = path.c_str();

    extract_faces_from_obj(file_path);
    return 0;
}
