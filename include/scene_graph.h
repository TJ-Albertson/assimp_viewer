/*-------------------------------------------------------------------------------\
scene.h

Functions to load, create and draw scene. 
Loads from json and creates a node tree.
Draws the node tree.

\-------------------------------------------------------------------------------*/
#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <model.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cjson/cJSON.h>

#include <collision.h>

struct SceneNode {
    char name[24];
    int id;

    Model* model;
    unsigned int shaderID;

    glm::mat4 m_modelMatrix;

    struct SceneNode* firstChild; // Pointer to the first child node
    struct SceneNode* nextSibling; // Pointer to the next sibling node
};

SceneNode* root_node;

unsigned int shaderIdArray[10];

void AddChild(SceneNode* parent, SceneNode* child);
int LoadScene(std::string const& path);
void DrawScene(SceneNode* root);
void DrawSceneNode(SceneNode* node, glm::mat4 parentTransform);

void AddChild(SceneNode* parent, SceneNode* child)
{
    if (parent == NULL || child == NULL) {
        return;
    }

    if (parent->firstChild == NULL) {
        parent->firstChild = child;
    } else {
        SceneNode* sibling = parent->firstChild;
        while (sibling->nextSibling != NULL) {
            sibling = sibling->nextSibling;
        }
        sibling->nextSibling = child;
    }
}


// Read json and load models
int LoadScene(std::string const& path)
{
    const char* filename = path.c_str();

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return 1;
    }

    // Calculate the size of the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate a buffer to store the file contents
    char* json_buffer = (char*)malloc(file_size + 1);
    if (json_buffer == NULL) {
        printf("Error allocating memory\n");
        fclose(file);
        return 1;
    }

    // Read the file into the buffer
    size_t bytes_read = fread(json_buffer, 1, file_size, file);
    json_buffer[bytes_read] = '\0'; // Null-terminate the buffer

    fclose(file);

    cJSON* root = cJSON_Parse(json_buffer);
 
    if (root == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        free(json_buffer);
        return 1;
    }

    int num_models = cJSON_GetArraySize(root);

    root_node = (SceneNode*)malloc(sizeof(SceneNode));
    strncpy(root_node->name, "root_node", sizeof(root_node->name));
    root_node->shaderID = 0;
    root_node->m_modelMatrix = glm::mat4(1.0f);
    root_node->firstChild = NULL;
    root_node->nextSibling = NULL;

    for (int i = 0; i < num_models; ++i) {

        cJSON* model = cJSON_GetArrayItem(root, i);

        SceneNode* node = (SceneNode*)malloc(sizeof(SceneNode));

        node->firstChild = NULL;
        node->nextSibling = NULL;

        strncpy(node->name, cJSON_GetObjectItem(model, "name")->valuestring, sizeof(node->name));
        node->shaderID = cJSON_GetObjectItem(model, "shaderId")->valueint;

        std::string path = cJSON_GetObjectItem(model, "filepath")->valuestring;
        std::string hitbox = cJSON_GetObjectItem(model, "hitbox")->valuestring;

        node->model = LoadModel(path);


        // it goes scale, rotation, translation. but you need to apply them in reverse like a stack i guess??
        /*
         * Note that we first do a translation and then a scale transformation when multiplying matrices. Matrix
         * multiplication is not commutative, which means their order is important. When multiplying matrices the right-
         * most matrix is first multiplied with the vector so you should read the multiplications from right to left. It is
         * advised to first do scaling operations, then rotations and lastly translations when combining matrices
         * otherwise they may (negatively) affect each other. For example, if you would first do a translation and then
         * scale, the translation vector would also scale!
         */

        glm::vec3 translation;
        glm::vec3 rotation;
        glm::vec3 scale;

        const char* translation_str = cJSON_GetObjectItem(model, "translation")->valuestring;
        sscanf(translation_str, "%f, %f, %f", &translation.x, &translation.y, &translation.z);

        const char* scale_str = cJSON_GetObjectItem(model, "scale")->valuestring;
        sscanf(scale_str, "%f, %f, %f", &scale.x, &scale.y, &scale.z);

        const char* rotation_str = cJSON_GetObjectItem(model, "rotation")->valuestring;
        sscanf(rotation_str, "%f, %f, %f", &rotation.x, &rotation.y, &rotation.z);

        glm::mat4 model_matrix = glm::mat4(1.0f);

        printf("translation: %f, %f, %f\n", translation.x, translation.y, translation.z);
        printf("scale: %f, %f, %f\n", scale.x, scale.y, scale.z);
        printf("rotation: %f, %f, %f\n", rotation.x, rotation.y, rotation.z);

        // Translation
        model_matrix = glm::translate(model_matrix, translation);

        // Rotation
        glm::quat rotationX = glm::angleAxis(glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::quat rotationY = glm::angleAxis(glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::quat rotationZ = glm::angleAxis(glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::quat finalRotation = rotationX * rotationY * rotationZ;
        glm::mat4 rotationMatrix = glm::toMat4(finalRotation);
        model_matrix = model_matrix * rotationMatrix;

        // Scale
        model_matrix = glm::scale(model_matrix, scale);

        node->m_modelMatrix = model_matrix;

        printf("Matrix:\n");
        for (int i = 0; i < 4; i++) {
            printf("[ ");
            for (int j = 0; j < 4; j++) {
                printf("%.2f ", glm::value_ptr(model_matrix)[i * 4 + j]);
            }
            printf("]\n");
        }

        CreateHitbox(hitbox, model_matrix);

        AddChild(root_node, node);
    }

    cJSON_Delete(root);
    free(json_buffer);

    return 0;
}


void DrawSceneNode(SceneNode* node, glm::mat4 parentTransform)
{
    // This will be changed in future. Plan is to use universal shader program.
    // From what I've read that is favorable due to the high cost of switching
    // shader programs.
    glUseProgram(shaderIdArray[node->shaderID]);

    glm::mat4 model = node->m_modelMatrix * parentTransform;

    setShaderMat4(shaderIdArray[node->shaderID], "model", model);

    DrawModel(node->model, shaderIdArray[node->shaderID]);

    SceneNode* child = node->firstChild;
    while (child != NULL) {
        DrawSceneNode(child, model);
        child = child->nextSibling;
    }
}

void DrawScene(SceneNode* root)
{
    glm::mat4 matrix = glm::mat4(1.0f);

    SceneNode* child = root->firstChild;
    while (child != NULL) {
        DrawSceneNode(child, matrix);
        child = child->nextSibling;
    }
}

void PrintTree(SceneNode* root)
{
    if (root == NULL) {
        return;
    }

    printf("Name: %s\n ", root->name);

    SceneNode* child = root->firstChild;
    while (child != NULL) {
        PrintTree(child);
        child = child->nextSibling;
    }

}

#endif