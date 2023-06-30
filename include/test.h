/*-------------------------------------------------------------------------------\
scene.h

Plan for this to hold scene children in set of nodes

\-------------------------------------------------------------------------------*/
#ifndef TEST_H
#define TEST_H

struct SceneNode {
    const char* name;
    int id;

    Model* model;

    int numChildren;
    SceneNode* children;
};

struct RootSceneNode {
    const char name[6];

    int numChildren;
    SceneNode* children;
};

RootSceneNode* rootNode;

void SearchForParentNode(SceneNode* node, int parentId, Model* model);
void AddNodeToScene(const int parentId, Model* model);

void InitSceneNode()
{
    rootNode->numChildren = 0;
    rootNode->children = (SceneNode*)malloc(sizeof(SceneNode));
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

            rootNode->children[0].name = model->m_Name;
            rootNode->children[0].id = 1;
            rootNode->children[0].model = model;
            rootNode->children[0].numChildren = 0;
            rootNode->children[0].children = (SceneNode*)malloc(sizeof(SceneNode));
        }
        else {

            int newSize = rootNode->numChildren++;
            SceneNode* newArray = (SceneNode*)malloc(newSize * sizeof(SceneNode));

            for (int i = 0; i < newSize - 1; i++) {
                newArray[i] = rootNode->children[i];
            }

            newArray[newSize].name = model->m_Name;
            newArray[newSize].id = 1;
            newArray[newSize].model = model;
            newArray[newSize].numChildren = 0;
            newArray[newSize].children = (SceneNode*)malloc(sizeof(SceneNode));

            rootNode->children = newArray;
        }
    }
    else {
        for (int i = 0; i < rootNode->numChildren; i++)
            SearchForParentNode(&rootNode->children[i], parentId, model);
    }
}

#endif