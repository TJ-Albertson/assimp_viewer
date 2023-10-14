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
#include <utils.h>

struct SceneNode {
    char name[24];
    char type[24];
    int id;

    Model* model;
    unsigned int shaderID;

    glm::mat4 m_modelMatrix;

    struct SceneNode* firstChild; // Pointer to the first child node
    struct SceneNode* nextSibling; // Pointer to the next sibling node
};

SceneNode* root_node;

unsigned int id;

unsigned int shaderIdArray[10];

SceneNode* CreateNode(SceneNode* parent, std::string const& path);
void AddChild(SceneNode* parent, SceneNode* child);

SceneNode* CreateTreeNode(cJSON* jsonNode);
SceneNode* BuildTree(cJSON* jsonNode);

int LoadScene(std::string const& path);

void DrawScene(SceneNode* root);
void DrawSceneNode(SceneNode* node, glm::mat4 parentTransform);

int generate_random_int(unsigned int address);

// Used in runtime model loading
SceneNode* CreateNode(SceneNode* parent, std::string const& path) {

    SceneNode* node = (SceneNode*)malloc(sizeof(SceneNode));

    node->firstChild = NULL;
    node->nextSibling = NULL;

    node->model = LoadModel(path);
    
    strncpy(node->type, "model", sizeof(node->type));
    strncpy(node->name, node->model->m_Name, sizeof(node->name));
    node->shaderID = 0;

    node->m_modelMatrix = glm::mat4(1.0f);

    CreateHitbox(path, glm::mat4(1.0f));

    AddChild(root_node, node);

    return node;
}

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

// Function to create a new tree node
SceneNode* CreateTreeNode(cJSON* jsonNode)
{
    SceneNode* node = (SceneNode*)malloc(sizeof(SceneNode));

    strncpy(node->name, cJSON_GetObjectItem(jsonNode, "name")->valuestring, sizeof(node->name));
    strncpy(node->type, cJSON_GetObjectItem(jsonNode, "type")->valuestring, sizeof(node->type));
    
    if (cJSON_GetObjectItem(jsonNode, "shaderId")) {
        node->shaderID = cJSON_GetObjectItem(jsonNode, "shaderId")->valueint;
    }
    
    std::string path = cJSON_GetObjectItem(jsonNode, "filepath")->valuestring;

    if (strcmp(node->type, "model") == 0) {
        node->model = LoadModel(filepath(path));
    }

    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale;

    const char* translation_str = cJSON_GetObjectItem(jsonNode, "translation")->valuestring;
    sscanf(translation_str, "%f, %f, %f", &translation.x, &translation.y, &translation.z);

    const char* scale_str = cJSON_GetObjectItem(jsonNode, "scale")->valuestring;
    sscanf(scale_str, "%f, %f, %f", &scale.x, &scale.y, &scale.z);

    const char* rotation_str = cJSON_GetObjectItem(jsonNode, "rotation")->valuestring;
    sscanf(rotation_str, "%f, %f, %f", &rotation.x, &rotation.y, &rotation.z);

    glm::mat4 model_matrix = glm::mat4(1.0f);

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

    if (strcmp(node->type, "hitbox") == 0) {
        CreateHitbox(filepath(path), model_matrix);
    }

    node->firstChild = NULL;
    node->nextSibling = NULL;

    int newId = generate_random_int((unsigned int)node);

    node->id = newId;

    return node;
}

// Function to build the tree structure recursively
SceneNode* BuildTree(cJSON* jsonNode)
{
    if (!jsonNode) {
        return NULL;
    }

    SceneNode* node = CreateTreeNode(jsonNode);

    cJSON* children = cJSON_GetObjectItem(jsonNode, "children");
    if (cJSON_IsArray(children)) {
        cJSON* child = children->child;
        if (child) {
            node->firstChild = BuildTree(child);
            SceneNode* sibling = node->firstChild;
            while ((child = child->next)) {
                sibling->nextSibling = BuildTree(child);
                sibling = sibling->nextSibling;
            }
        }
    }

    return node;
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

    id = 0;

    root_node = (SceneNode*)malloc(sizeof(SceneNode));
    strncpy(root_node->name, "root_node", sizeof(root_node->name));
    root_node->shaderID = 0;
    root_node->m_modelMatrix = glm::mat4(1.0f);
    root_node->firstChild = NULL;
    root_node->nextSibling = NULL;
    root_node->id = id;

    for (int i = 0; i < num_models; ++i)
    {
        cJSON* model = cJSON_GetArrayItem(root, i);

        SceneNode* node = (SceneNode*)malloc(sizeof(SceneNode));

        node = BuildTree(model);

        AddChild(root_node, node);
    }

    cJSON_Delete(root);
    free(json_buffer);

    return 0;
}


void DrawSceneNode(SceneNode* node, glm::mat4 parentTransform)
{
    glm::mat4 model = node->m_modelMatrix * parentTransform;

    if (strcmp(node->type, "model") == 0) {
        // This will be changed in future. Plan is to use universal shader program.
        // From what I've read that is favorable due to the high cost of switching
        // shader programs.
        
        glUseProgram(shaderIdArray[node->shaderID]);

        setShaderMat4(shaderIdArray[node->shaderID], "model", model);

        DrawModel(node->model, shaderIdArray[node->shaderID]);
    }

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

int generate_random_int(unsigned int address)
{
    srand(address);

    // Generate and print a random number
    int randomNumber = rand();

    return randomNumber;
}


#endif