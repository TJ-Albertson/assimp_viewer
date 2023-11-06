/*-------------------------------------------------------------------------------\
scene.h

Functions to load, create and draw scene. 
Loads from json and creates a node tree.
Draws the node tree.

Builds a tree from every top level node in the json array, then adds that node to
the root node.

\-------------------------------------------------------------------------------*/
#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <model.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cjson/cJSON.h>

#include <collision.h>
#include <utils.h>
#include <my_math.h>

#include "shader_m.h"

struct SceneNode {
    char name[24];
    char type[24];
    int id;

    Model* model;
    unsigned int shaderID;

    Hitbox hitbox;

    // Global Position
    glm::mat4 m_modelMatrix;

    // Local Position
    glm::vec3 m_pos = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_eulerRot = { 0.0f, 0.0f, 0.0f }; // In degrees
    glm::vec3 m_scale = { 1.0f, 1.0f, 1.0f };

    //need to change l8r
    bool dirty_flag = false;

    struct SceneNode* firstChild; // Pointer to the first child node
    struct SceneNode* nextSibling; // Pointer to the next sibling node
};

SceneNode* root_node;
unsigned int id;
unsigned int shaderIdArray[10];

bool drawHitboxes = false;

SceneNode* CreateNode(SceneNode* parent, std::string const& path);
void AddChild(SceneNode* parent, SceneNode* child);

SceneNode* CreateTreeNode(cJSON* jsonNode, glm::mat4 matrix);
SceneNode* BuildTree(cJSON* jsonNode, glm::mat4 matrix);

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

// Used to add trees to rootnode
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
SceneNode* CreateTreeNode(cJSON* jsonNode, glm::mat4 matrix)
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

    node->m_pos = translation;
    node->m_eulerRot = rotation;
    node->m_scale = scale;

    glm::mat4 localMatrix = glm::mat4(1.0f);

    localMatrix = glm::translate(localMatrix, node->m_pos);

    localMatrix = my_rotation(localMatrix, node->m_eulerRot);

    localMatrix = glm::scale(localMatrix, node->m_scale);

    node->m_modelMatrix = matrix * localMatrix;

    if (strcmp(node->type, "hitbox") == 0) {
        AABB_node* aabb_node = CreateHitbox(filepath(path), node->m_modelMatrix);
        node->model = LoadAABB_Model(aabb_node);

        Hitbox hitbox;
        hitbox.m_Matrix = &(node->m_modelMatrix);
        hitbox.rootAABB = aabb_node;
        hitboxes.push_back(hitbox);
    }

    node->firstChild = NULL;
    node->nextSibling = NULL;

    int newId = generate_random_int((unsigned int)node);

    node->id = newId;

    return node;
}

// Function to build the tree structure recursively
SceneNode* BuildTree(cJSON* jsonNode, glm::mat4 matrix)
{
    if (!jsonNode) {
        return NULL;
    }

    SceneNode* node = CreateTreeNode(jsonNode, matrix);

    cJSON* children = cJSON_GetObjectItem(jsonNode, "children");

    if (cJSON_IsArray(children)) {

        cJSON* child = children->child;

        if (child) {

            node->firstChild = BuildTree(child, node->m_modelMatrix);
            SceneNode* sibling = node->firstChild;

            while ((child = child->next)) {

                sibling->nextSibling = BuildTree(child, node->m_modelMatrix);
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

        node = BuildTree(model, root_node->m_modelMatrix);

        AddChild(root_node, node);
    }

    cJSON_Delete(root);
    free(json_buffer);

    return 0;
}


void DrawSceneNode(SceneNode* node, glm::mat4 parentTransform)
{
    glm::mat4 model = node->m_modelMatrix;

    if (node->dirty_flag) {

        glm::mat4 localMatrix = glm::mat4(1.0f);

        localMatrix = glm::translate(localMatrix, node->m_pos);

        localMatrix = my_rotation(localMatrix, node->m_eulerRot);

        localMatrix = glm::scale(localMatrix, node->m_scale);

        model = parentTransform * localMatrix; // node->m_modelMatrix * parentTransform;

        node->m_modelMatrix = model;

        node->dirty_flag = false;
    }

    if (strcmp(node->type, "model") == 0) {
        // This will be changed in future. Plan is to use universal shader program.
        // From what I've read that is favorable due to the high cost of switching
        // shader programs.

        glUseProgram(shaderIdArray[node->shaderID]);
        setShaderMat4(shaderIdArray[node->shaderID], "model", model);

        DrawModel(node->model, shaderIdArray[node->shaderID]);
    }

    if (strcmp(node->type, "hitbox") == 0 && drawHitboxes)
{
        
         glUseProgram(shaderIdArray[1]);
        /*
        if (node.id is in collisionMap) {
            set hitbox shader color green
        }
        
        
        */

        for (const unsigned int& element : colliding_aabbs)
        {
            if (element == node->id) {
                printf("colliding\n");
                setShaderVec4(shaderIdArray[1], "color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

            }
        }
        //setShaderVec4(shaderIdArray[1], "color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

        setShaderMat4(shaderIdArray[1], "model", model);

        DrawAABB_Model(node->model, shaderIdArray[1]);
    }

    SceneNode* child = node->firstChild;
    while (child != NULL) {
        DrawSceneNode(child, node->m_modelMatrix);
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


void MarkChildNodes(SceneNode* node)
{
    node->dirty_flag = true;

    SceneNode* child = node->firstChild;
    while (child != NULL) {
        MarkChildNodes(child);
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