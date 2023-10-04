/*-------------------------------------------------------------------------------\
scene.h

Plan for this to hold scene children in set of nodes

\-------------------------------------------------------------------------------*/
#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <model.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cjson/cJSON.h>

#include <collision.h>

struct Orientation {
    glm::vec3 m_Pos = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_Rot = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_Scale = { 1.0f, 1.0f, 1.0f };
};

struct SceneNode {
    char name[24];
    int id;

    Model* model;
    unsigned int shaderID;
    Orientation* orientation;

    glm::mat4 m_modelMatrix;

    int numChildren;
    SceneNode* children;

    struct SceneNode* firstChild; // Pointer to the first child node
    struct SceneNode* nextSibling; // Pointer to the next sibling node
};

struct RootSceneNode {
    char name[6];

    int numChildren;
    SceneNode* children;
};

RootSceneNode* rootNode;

SceneNode* nodes;

SceneNode* root_node;

unsigned int shaderIdArray[10];

void InitSceneNode();
void SearchForParentNode(SceneNode* node, int parentId, Model* model);
void DrawScene(SceneNode* root);
void DrawSceneNode(SceneNode* node, glm::mat4 parentTransform);

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



SceneNode* createNode(SceneNode node) {

    SceneNode* newNode = (SceneNode*)malloc(sizeof(SceneNode));

    if (newNode == NULL) {
        perror("Failed to allocate memory for a new node");
        exit(1);
    }

    strncpy(newNode->name, node.name, sizeof(newNode->name) - 1);
    newNode->name[sizeof(newNode->name) - 1] = '\0';
    newNode->model = node.model;
    newNode->shaderID = node.shaderID;
    newNode->m_modelMatrix = node.m_modelMatrix;

    newNode->firstChild = NULL;
    newNode->nextSibling = NULL;
    return newNode;
}

void addChild(SceneNode* parent, SceneNode* child)
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

void printTree(SceneNode* root)
{
    if (root == NULL) {
        return;
    }

    printf("Name: %s\n ", root->name);

    SceneNode* child = root->firstChild;
    while (child != NULL) {
        printTree(child);
        child = child->nextSibling;
    }
}

void SearchForParentNode(SceneNode* node, int parentId, Model* model) {

    if (parentId == node->id) {
        if (node->numChildren == 0) {

            //node->children[0].name = model->m_Name;
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

            //newArray[newSize].name = model->m_Name;
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
    //newNode.name = (char*)malloc(strlen(model->m_Name) + 1); // +1 for null-terminator
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

            //newNode.name = (char*)malloc(strlen(model->m_Name) * sizeof(char));
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

            //new_array[newSize - 1].name = (char*)malloc(strlen(model->m_Name) * sizeof(char));
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

    // Now access JSON elements
    cJSON* name = cJSON_GetObjectItem(root, "filepath");
    if (cJSON_IsString(name)) {
        printf("filepath: %s\n", name->valuestring);
    }

    cJSON* shaderId = cJSON_GetObjectItem(root, "shaderId");
    if (cJSON_IsNumber(shaderId)) {
        printf("shaderId: %d\n", shaderId->valueint);
    }

    //nodes = (SceneNode*)malloc(num_models * sizeof(SceneNode));


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

        CreateHitbox(hitbox, model_matrix);

        addChild(root_node, node);
    }

    /*
    for (int i = 0; i < num_models; ++i) {
        printf("{\n");
        printf("            Name: %s\n", nodes[i].name);
        printf("        shaderID: %d\n", nodes[i].shaderID);
        printf("    matrix[0][0]: %f\n", nodes[i].m_modelMatrix[0][0]);
        printf("}\n");
    }
    */

    printTree(root_node);

    cJSON_Delete(root);
    free(json_buffer);

    return 0;
}


// it goes scale, rotation, translation. but you need to apply them in reverse like a stack i guess??
/*
 * Note that we first do a translation and then a scale transformation when multiplying matrices. Matrix
 * multiplication is not commutative, which means their order is important. When multiplying matrices the right-
 * most matrix is first multiplied with the vector so you should read the multiplications from right to left. It is
 * advised to first do scaling operations, then rotations and lastly translations when combining matrices
 * otherwise they may (negatively) affect each other. For example, if you would first do a translation and then
 * scale, the translation vector would also scale!
 */
void DrawSceneNode(SceneNode* node, glm::mat4 parentTransform)
{
    //glUseProgram(shaderIdArray[node->shaderID]);

    //try printing mat
    glm::mat4 model = node->m_modelMatrix; //* parentTransform;

    setShaderMat4(node->shaderID, "model", model);

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

#endif