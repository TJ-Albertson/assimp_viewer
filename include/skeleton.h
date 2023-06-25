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

std::vector<std::string> BoneNames;

int BoneID = 0;

struct BoneStruct {
    int ID;
    glm::mat4 Offset;
};

std::map<std::string, BoneStruct> BoneMap;

int nodeId = 0;

aiNode* aiRootNode;

struct SkeletonNode {
    char* m_NodeName;
    int id;

    glm::mat4 m_Transformation;
    glm::mat4 m_Offset;

    unsigned int m_NumChildren;
    SkeletonNode** m_Children;
};

void BoneCheckRoot(aiNode* node, const aiScene* scene);
void BoneCheck(aiNode* node, const aiScene* scene);
void BoneCheckParents(aiNode* boneNode, aiNode* meshNode);
void CreateBoneMap(aiNode* node, const aiScene* scene);

void CreateBoneMap2(const aiScene* scene);

void CreateSkeleton(const aiNode* node, SkeletonNode* skeletonNode);
SkeletonNode* copyNodeTree(const aiNode* root);

    // Create BoneMap
    // Find all necessary Nodes for skeleton (not all nodes necessary are bones)
    // Create Copy of node hiearchy with only necessary nodes for animation
    // Copy nodes will have additional information( BoneID and Offset); If node is not bone then id == -1

void printNodes(SkeletonNode* node)
{
    if (node == NULL) {
        return;
    }

    if (node->m_NodeName == NULL) {
        return;
    }

    std::cout << "[ " << node->m_NodeName << std::endl;
    std::cout << "              id: " << node->id << std::endl;
    std::cout << "     numChildren: " << node->m_NumChildren << std::endl;
    std::cout << "] " << std::endl;
    std::cout << " " << std::endl;

    if (node->m_NumChildren > 0) {
        for (int i = 0; i < node->m_NumChildren; ++i)
            printNodes(node->m_Children[i]);
    }
}


SkeletonNode* LoadSkeleton(const aiScene* scene)
{
    //Assimp::Importer importer;

    // "C:/Users/tjalb/source/repos/game/resources/objects/vampire/dancing_vampire.dae"
    //const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

    // Used in BoneCheck to find Node associated with Bone
    aiRootNode = scene->mRootNode;

    //CreateBoneMap(scene->mRootNode, scene);
    CreateBoneMap2(scene);

    BoneCheckRoot(scene->mRootNode, scene);

    // SkeletonNode* skeletonRootNode = (SkeletonNode *)malloc(sizeof(SkeletonNode));
    // CreateSkeleton(scene->mRootNode, skeletonRootNode);

    SkeletonNode* skeletonRootNode = copyNodeTree(scene->mRootNode);    

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

int isStringInBoneVectorAndIndex(const std::string& target) {
    int i = 0;
    for (const auto& str : BoneNames) {
        if (str == target) {
            return i;
        }
        i++;
    }
    return -1;
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


//For every Mesh in Node; For every Bone in Mesh; Add bone to map


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

void FindSkeletonRoot();
// idea. same as bonecheck/bonecheckroot

// this is bad i think? after first skeleton node all following are bones
// Is the Skeleton ususally the child of the root node?
// Are there any children in the skelton hierachy that arent't bones?


void CreateSkeleton(const aiNode* node, SkeletonNode* skeletonNode)
{

	std::string nodeName = std::string(node->mName.C_Str());
	int index = -1;
    glm::mat4 offset = glm::mat4(1.0f);

	if (BoneMap.find(nodeName) != BoneMap.end()) {
		index = BoneMap[nodeName].ID;
        offset = BoneMap[nodeName].Offset;
	}

    

    //skeletonNode->m_NodeName = node->mName.C_Str();
    skeletonNode->m_NumChildren = node->mNumChildren;

    skeletonNode->id = index;
    skeletonNode->m_Offset = offset;

    skeletonNode->m_Transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(node->mTransformation);

    int numChildren = node->mNumChildren;

    if (numChildren > 0) {
        skeletonNode->m_Children = (SkeletonNode**)malloc(numChildren * sizeof(SkeletonNode*));

        for (int i = 0; i < numChildren; ++i) {

            SkeletonNode* newBone = (SkeletonNode*)malloc(sizeof(SkeletonNode));

            
            CreateSkeleton(node->mChildren[i], newBone);
            skeletonNode->m_Children[i] = newBone;


        }
    } else {
        skeletonNode->m_Children = NULL;
    }
}


SkeletonNode* createNode(const aiNode* node, int mNumChildren)
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

    size_t nameLength = node->mName.length;

    newNode->m_NodeName = (char*)malloc(nameLength * sizeof(char));
    
    std::strcpy(newNode->m_NodeName, nodemName);

    newNode->m_NumChildren = mNumChildren;
    newNode->id = index;
    newNode->m_Offset = offset;
    newNode->m_Transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(node->mTransformation);

    newNode->m_Children = (SkeletonNode**)malloc(mNumChildren * sizeof(SkeletonNode*));

    return newNode;
}

SkeletonNode* copyNodeTree(const aiNode* root)
{
    if (root == NULL) {
        return NULL;
    }

    SkeletonNode* newRoot = createNode(root, root->mNumChildren);

    for (int i = 0; i < root->mNumChildren; i++) {
        newRoot->m_Children[i] = copyNodeTree(root->mChildren[i]);
    }

    return newRoot;
}

#endif