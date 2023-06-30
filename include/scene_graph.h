/*-------------------------------------------------------------------------------\
scene.h

Plan for this to hold scene children in set of nodes

\-------------------------------------------------------------------------------*/
#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <model.h>;

struct SceneNode {
    char* name;
    int id;

    Model* model;

    int numChildren;
    SceneNode* children;
};

struct RootSceneNode {
    char name[6];

    int numChildren;
    SceneNode* children;
};

RootSceneNode* rootNode;

void SearchForParentNode(SceneNode* node, int parentId, Model* model);

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

void AddNodeToScene(const int parentId, Model* model) {

    bool parentIsRoot = (parentId == 0);

    if (parentIsRoot) {
        if (rootNode->numChildren == 0) {

            SceneNode newNode;

            newNode.name = (char*)malloc(strlen(model->m_Name) * sizeof(char));
            strcpy(newNode.name, model->m_Name);
           
            newNode.id = 1;
            newNode.model = model;
            newNode.numChildren = 0;
            newNode.children = (SceneNode*)malloc(sizeof(SceneNode));

            rootNode->children[0] = newNode;
            rootNode->numChildren++;
        }
        else {

            int newSize = rootNode->numChildren + 1;

            SceneNode* new_array = (SceneNode*)realloc(rootNode->children, newSize * sizeof(SceneNode));

            new_array[newSize - 1].name = (char*)malloc(strlen(model->m_Name) * sizeof(char));
            strcpy(new_array[newSize - 1].name, model->m_Name);

            new_array[newSize - 1].id = 1;
            new_array[newSize - 1].model = model;
            new_array[newSize - 1].numChildren = 0;
            new_array[newSize - 1].children = (SceneNode*)malloc(sizeof(SceneNode));
            
            rootNode->children = new_array;
            rootNode->numChildren++;
        }
    }
    else {
        for (int i = 0; i < rootNode->numChildren; i++)
            SearchForParentNode(&rootNode->children[i], parentId, model);
    }
}

#endif