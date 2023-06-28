/*-------------------------------------------------------------------------------\
scene.h

Plan for this to hold scene children in set of nodes

\-------------------------------------------------------------------------------*/
#ifndef SCENE_H
#define SCENE_H

#include <model.h>;

struct SceneNode {
    const char* name;
    int id;

    Model* model;

    int numChildren;
    SceneNode* children;
};

struct RootSceneNode {
    const char name[6] = "Scene";

    int numChildren;
    SceneNode* children;
};

RootSceneNode* rootNode;

void InitSceneNode()
{
    rootNode = (RootSceneNode*)malloc(sizeof(RootSceneNode));
    rootNode->numChildren = 0;
    rootNode->children = (SceneNode*)malloc(sizeof(SceneNode));
}

void SearchForParentNode(SceneNode node, const int parentId, Model* model) {
    
    if (parentId == node.id) {

    } else {
        for (int i = 0; i < rootNode->numChildren; i++)
            SearchForParentNode(rootNode->children[0], parentId, model);
    }
}

void AddNodeToScene(const int parentId, Model* model) {

    if (parentId == 0) {
        if (rootNode->numChildren == 0) {

            rootNode->children[0].name = model->m_Name;
            rootNode->children[0].id = 1;
            rootNode->children[0].model = model;
            rootNode->children[0].numChildren = 0;
            rootNode->children[0].children = (SceneNode*)malloc(sizeof(SceneNode));
        } else {
            int newSize = rootNode->numChildren++;
            SceneNode* newArray = (SceneNode*)malloc(newSize * sizeof(SceneNode));

            for (int i = 0; i < newSize - 1; i++) {
                newArray[i] = rootNode->children[i];
            }
        }
    } else {
        for (int i = 0; i < rootNode->numChildren; i++)
            SearchForParentNode(rootNode->children[0], parentId, model);
    }
}

#endif