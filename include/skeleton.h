/*-------------------------------------------------------------------------------\
skeleton.h

Functions: 
    Create copy of node hierachy with only nodes necessary for skeletion animation
    Add additional attributes to nodes for more efficient animatino (m_Offset and id)
    Create map of bone id's to be used in vertex data (so the vertex knows which bone transform to apply in shader program) 

\-------------------------------------------------------------------------------*/

#ifndef SKELETON_H
#define SKELETON_H

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <assimp_glm_helpers.h>

struct BoneStruct {
    int ID;
    glm::mat4 Offset;
};

struct SkeletonNode {
    char* m_NodeName;
    int id;

    glm::mat4 m_Transformation;
    glm::mat4 m_Offset;

    unsigned int m_NumChildren;
    SkeletonNode** m_Children;
};

std::vector<std::string> BoneNames;
std::map<std::string, BoneStruct> BoneMap;
int BoneID = 0;

// For aiNode.FindNode(const char *name). Used to find Node in hierarchy = aiBone.mName
aiNode* aiRootNode;

void BoneCheckRoot(aiNode* node, const aiScene* scene);
void BoneCheck(aiNode* node, const aiScene* scene);
void BoneCheckParents(aiNode* boneNode, aiNode* meshNode);

void CreateBoneMap(aiNode* node, const aiScene* scene);
void CreateBoneMap2(const aiScene* scene);

SkeletonNode* CreateNode(const aiNode* node, int mNumChildren);
SkeletonNode* CopyNodeTree(const aiNode* root);

SkeletonNode* LoadSkeleton(const aiScene* scene)
{
    // Used in BoneCheck to find Node associated with Bone
    aiRootNode = scene->mRootNode;

    CreateBoneMap(aiRootNode, scene);

    BoneCheckRoot(scene->mRootNode, scene);

    SkeletonNode* skeletonRootNode = CopyNodeTree(scene->mRootNode);  

     

    return skeletonRootNode;
}


bool isStringInBoneVector(const std::string& target) {
    for (const auto& str : BoneNames) {
        if (str == target) {
            return true;
        }
    }
    return false;
}

void BoneCheckParents(aiNode* boneNode, aiNode* meshNode)
{
    // if node.parent == mesh node or mesh node.parent then skip
    aiString boneParentName = boneNode->mParent->mName;

    //need new clause to stop if bone already marked
    if (!isStringInBoneVector(boneParentName.C_Str())) {
        if (boneParentName != meshNode->mName && boneParentName != meshNode->mParent->mName)
        {
            BoneNames.push_back(boneNode->mParent->mName.C_Str());
            BoneCheckParents(boneNode->mParent, meshNode);
        }
        else {
            BoneNames.push_back(boneNode->mParent->mName.C_Str());
        }
    }

}

void BoneCheckRoot(aiNode* node, const aiScene* scene)
{
    if (node->mNumMeshes > 0) {

        int nNumMeshes = node->mNumMeshes;

        for (int i = 0; i < nNumMeshes; ++i) {

            int meshIndex = node->mMeshes[i];
            int numBones = scene->mMeshes[meshIndex]->mNumBones;

            for (int j = 0; j < numBones; ++j) {
                BoneNames.push_back(scene->mMeshes[meshIndex]->mBones[j]->mName.C_Str());
            }
        }
    }
    else {
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        BoneCheck(node->mChildren[i], scene);
    }
}

void BoneCheck(aiNode* node, const aiScene* scene)
{
    if (node->mNumMeshes > 0) {

        for (int i = 0; i < node->mNumMeshes; i++) {

            int meshIndex = node->mMeshes[i];

            for (int j = 0; j < scene->mMeshes[meshIndex]->mNumBones; j++) {

                aiString boneNodeName = scene->mMeshes[meshIndex]->mBones[j]->mName;

                if (!isStringInBoneVector(boneNodeName.C_Str())) {
                    BoneNames.push_back(boneNodeName.C_Str());
                    aiNode* boneNode = aiRootNode->FindNode(boneNodeName);
                    BoneCheckParents(boneNode, node);
                }
            }
        }
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        BoneCheck(node->mChildren[i], scene);
    }
}

// change to bone struct and add offset
void CreateBoneMap(aiNode* node, const aiScene* scene) {
    if (node->mNumMeshes > 0) {

        for (int i = 0; i < node->mNumMeshes; ++i) {

            int meshIndex = node->mMeshes[i];

            for (int j = 0; j < scene->mMeshes[meshIndex]->mNumBones; ++j) {

                aiString boneNodeName = scene->mMeshes[meshIndex]->mBones[j]->mName;

                if (BoneMap.find(boneNodeName.C_Str()) == BoneMap.end()) {

                    aiMatrix4x4 offset = scene->mMeshes[meshIndex]->mBones[j]->mOffsetMatrix;

                    glm::mat4 glm_offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(offset);

                    BoneStruct newBone = { BoneID++, glm_offset };

                    BoneMap[boneNodeName.C_Str()] = newBone;
                }
            }
        }
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        CreateBoneMap(node->mChildren[i], scene);
    }
}

void CreateBoneMap2(const aiScene* scene) {

    int mNumMeshes = scene->mNumMeshes;

    for (int i = 0; i < mNumMeshes; ++i) {
       
        aiMesh* mesh = scene->mMeshes[i];

        int mNumBones = mesh->mNumBones;

        for (int j = 0; j < mNumBones; ++j) {
            
            aiBone* bone = mesh->mBones[j];

            std::string boneName = bone->mName.C_Str();

            if (BoneMap.find(boneName) == BoneMap.end()) {
                
                aiMatrix4x4 offset = bone->mOffsetMatrix;

                glm::mat4 glm_offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(offset);

                BoneStruct newBone = { BoneID++, glm_offset };

                BoneMap[boneName] = newBone;
            }
        }
    }

    /*
    aiAnimation* aiAnimation = scene->mAnimations[0];

    int mNumChannels = aiAnimation->mNumChannels;

    for (int i = 0; i < mNumChannels; ++i) {

        aiNodeAnim* aiNodeAnim = aiAnimation->mChannels[i];

        std::string boneName = aiNodeAnim->mNodeName.C_Str();

        if (BoneMap.find(boneName) == BoneMap.end())
        {
            BoneMap[boneName].ID = BoneID++;
        }
    }
    */
}

SkeletonNode* CreateNode(const aiNode* node, int mNumChildren)
{
    SkeletonNode* newNode = (SkeletonNode*)malloc(sizeof(SkeletonNode));

    std::string nodeName = std::string(node->mName.C_Str());
	int index = -1;
    glm::mat4 offset = glm::mat4(1.0f);

	if (BoneMap.find(nodeName) != BoneMap.end()) {
        index = BoneMap[nodeName].ID;
        offset = BoneMap[nodeName].Offset;
	}

    const char* nodemName = node->mName.C_Str();
    size_t nameLength     = node->mName.length;

    newNode->m_NodeName = (char*)malloc(nameLength * sizeof(char));
    std::strcpy(newNode->m_NodeName, nodemName);

    newNode->m_NumChildren = mNumChildren;
    newNode->id = index;
    newNode->m_Offset = offset;
    newNode->m_Transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(node->mTransformation);

    newNode->m_Children = (SkeletonNode**)malloc(mNumChildren * sizeof(SkeletonNode*));

    return newNode;
}

SkeletonNode* CopyNodeTree(const aiNode* root)
{
    if (root == NULL) {
        return NULL;
    }

    SkeletonNode* newRoot = CreateNode(root, root->mNumChildren);

    for (int i = 0; i < root->mNumChildren; i++) {
        newRoot->m_Children[i] = CopyNodeTree(root->mChildren[i]);
    }

    return newRoot;
}

#endif