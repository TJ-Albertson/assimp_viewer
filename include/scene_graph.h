/*-------------------------------------------------------------------------------\
scene.h

Plan for this to hold scene children in set of nodes

\-------------------------------------------------------------------------------*/
#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <model.h>;
#include <glm/gtc/matrix_transform.hpp>

#include <cjson/cJSON.h>

struct Orientation {
    glm::vec3 m_Pos = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_Rot = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_Scale = { 1.0f, 1.0f, 1.0f };
};

struct SceneNode {
    char* name;
    int id;

    Model* model;
    unsigned int shaderID;
    Orientation* orientation;


    int numChildren;
    SceneNode* children;
};

struct RootSceneNode {
    char name[6];

    int numChildren;
    SceneNode* children;
};

RootSceneNode* rootNode;

void InitSceneNode();
void SearchForParentNode(SceneNode* node, int parentId, Model* model);
void DrawScene();
void DrawSceneNode(SceneNode node, glm::mat4 parentTransform);

//void AddNodeToScene(const int parentId, Model* model, unsigned int shaderID, Orientation* orientation);
void AddNodeToScene(const int parentId, Model* model, unsigned int shaderID);
void AddChildNode(SceneNode* parentNode, SceneNode newNode);


int LoadScene(std::string const& path);

//change to allocate scene space 10 at a time
void InitSceneNode()
{
    rootNode = (RootSceneNode*)malloc(sizeof(RootSceneNode));
    rootNode->numChildren = 0;
    rootNode->children = (SceneNode*)malloc(sizeof(SceneNode));
    strcpy(rootNode->name, "Scene");
}

void SearchForParentNode(SceneNode* node, int parentId, Model* model) {

    if (parentId == node->id) {
        if (node->numChildren == 0) {

            node->children[0].name = model->m_Name;
            node->children[0].id = 1;
            node->children[0].model = model;
            node->children[0].numChildren = 0;
            node->children[0].children = (SceneNode*)malloc(sizeof(SceneNode));
        }
        else {

            int newSize = node->numChildren++;
            SceneNode* newArray = (SceneNode*)malloc(newSize * sizeof(SceneNode));

            for (int i = 0; i < newSize - 1; i++) {
                newArray[i] = node->children[i];
            }

            newArray[newSize].name = model->m_Name;
            newArray[newSize].id = 1;
            newArray[newSize].model = model;
            newArray[newSize].numChildren = 0;
            newArray[newSize].children = (SceneNode*)malloc(sizeof(SceneNode));

            node->children = newArray;
        }
    }
    else {
        for (int i = 0; i < node->numChildren; i++)
            SearchForParentNode(&node->children[i], parentId, model);
    }
}




void FindParentNode(SceneNode* node, int parentId, SceneNode newNode)
{
    if (node->id == parentId) {
        AddChildNode(node, newNode);
    }

    for (int i = 0; i < node->numChildren; i++) {
        FindParentNode(&node->children[i], parentId, newNode);
    }
}
void AddChildNodeRoot(RootSceneNode* parentNode, SceneNode newNode)
{
    int newSize = parentNode->numChildren + 1;

    SceneNode* new_array = (SceneNode*)realloc(parentNode->children, newSize * sizeof(SceneNode));
    if (new_array == NULL) {
        // Handle memory allocation failure
        return;
    }

    new_array[newSize - 1] = newNode;
    parentNode->children = new_array;
    parentNode->numChildren = newSize;
}

void AddChildNode(SceneNode* parentNode, SceneNode newNode)
{
    int newSize = parentNode->numChildren + 1;

    SceneNode* new_array = (SceneNode*)realloc(parentNode->children, newSize * sizeof(SceneNode));
    if (new_array == NULL) {
        // Handle memory allocation failure
        return;
    }

    new_array[newSize - 1] = newNode;
    parentNode->children = new_array;
    parentNode->numChildren = newSize;
}

// pass parent id and node data. will add as child to parent with parent id
void zzzAddNodeToScene(const int parentId, Model* model, unsigned int shaderID, Orientation* orientation)
{
    SceneNode newNode;
    newNode.name = (char*)malloc(strlen(model->m_Name) + 1); // +1 for null-terminator
    strcpy(newNode.name, model->m_Name);
    newNode.id = 1;
    newNode.model = model;
    newNode.shaderID = shaderID;
    newNode.orientation = orientation;
    newNode.numChildren = 0;
    newNode.children = NULL; // Initialize to NULL, will allocate as needed

    if (parentId == 0) {
        AddChildNodeRoot(rootNode, newNode);
    } else {
        for (int i = 0; i < rootNode->numChildren; i++) {
            FindParentNode(&rootNode->children[i], parentId, newNode);
        }
    }
}





void AddNodeToScene(const int parentId, Model* model, unsigned int shaderID)
{

    bool parentIsRoot = (parentId == 0);

    if (parentIsRoot) {
        if (rootNode->numChildren == 0) {

            SceneNode newNode;

            newNode.name = (char*)malloc(strlen(model->m_Name) * sizeof(char));
            strcpy(newNode.name, model->m_Name);

            newNode.id = 1;
            newNode.model = model;
            newNode.numChildren = 0;
            newNode.shaderID = shaderID;
            newNode.children = (SceneNode*)malloc(sizeof(SceneNode));

            rootNode->children[0] = newNode;
            rootNode->numChildren++;
        } else {

            int newSize = rootNode->numChildren + 1;

            SceneNode* new_array = (SceneNode*)realloc(rootNode->children, newSize * sizeof(SceneNode));

            new_array[newSize - 1].name = (char*)malloc(strlen(model->m_Name) * sizeof(char));
            strcpy(new_array[newSize - 1].name, model->m_Name);

            new_array[newSize - 1].id = 1;
            new_array[newSize - 1].model = model;
            new_array[newSize - 1].shaderID = shaderID;
            new_array[newSize - 1].numChildren = 0;
            new_array[newSize - 1].children = (SceneNode*)malloc(sizeof(SceneNode));

            rootNode->children = new_array;
            rootNode->numChildren++;
        }
    } else {
        for (int i = 0; i < rootNode->numChildren; i++)
            SearchForParentNode(&rootNode->children[i], parentId, model);
    }
}

// Read json and load models
int LoadScene(std::string const& path)
{
    const char* filename = path.c_str(); // Replace with your JSON file's name

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

    // Close the file
    fclose(file);

    // Now parse the JSON data
    cJSON* root = cJSON_Parse(json_buffer);
    if (root == NULL) {
        // Handle parsing error
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        free(json_buffer);
        return 1;
    }

    // Now you can access JSON elements
    cJSON* name = cJSON_GetObjectItem(root, "filepath");
    if (cJSON_IsString(name)) {
        printf("filepath: %s\n", name->valuestring);
    }

    cJSON* shaderId = cJSON_GetObjectItem(root, "shaderId");
    if (cJSON_IsNumber(shaderId)) {
        printf("shaderId: %d\n", shaderId->valueint);
    }

    // Don't forget to free the cJSON structure and the buffer when you're done with them
    cJSON_Delete(root);
    free(json_buffer);

    return 0;
}

void DrawScene() {

    glm::mat4 matrix = glm::mat4(1.0f);
    
    for (int i = 0; i < rootNode->numChildren; ++i) {
        DrawSceneNode(rootNode->children[i], matrix);
    }
}

void DrawSceneNode(SceneNode node, glm::mat4 parentTransform) {

    glm::vec3 position = node.orientation->m_Pos;
    glm::vec3 rotation = node.orientation->m_Rot;
    glm::vec3 scale    = node.orientation->m_Scale;

    glm::mat4 model = glm::mat4(1.0f);

    glUseProgram(node.shaderID);

    //model = glm::translate(model, node.orientation->m_Pos);
    //model = glm::scale(model, node.orientation->m_Scale);
    //model = glm::rotate(model, node.orientation->m_Rot);

    const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // Y * X * Z
    const glm::mat4 rotationMatrix = transformY * transformX * transformZ;

    // translation * rotation * scale (also know as TRS matrix)
    model = glm::translate(glm::mat4(1.0f), position) * rotationMatrix * glm::scale(glm::mat4(1.0f), scale);

    setShaderMat4(node.shaderID, "model", model);

    DrawModel(node.model, node.shaderID);
    
    for (int i = 0; i < node.numChildren; ++i) {
        DrawSceneNode(node.children[i], model);
    }
}


#endif