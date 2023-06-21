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

struct Bone {
    int ID;
    glm::mat4 Offset;
};

std::map<std::string, Bone> BoneMap;

int nodeId = 0;

aiNode* aiRootNode;

struct SkeletonNode {
    std::string m_NodeName;
    int id;

    glm::mat4 m_Transformation;
    glm::mat4 m_Offset;

    unsigned int m_NumChildren;
    std::vector<SkeletonNode> m_Children;
};

void BoneCheckRoot(aiNode* node, const aiScene* scene);
void BoneCheck(aiNode* node, const aiScene* scene);
void BoneCheckParents(aiNode* boneNode, aiNode* meshNode);


void CreateSkeleton(const aiNode* node, SkeletonNode& skeletonNode);


    // Create BoneMap
    // Find all necessary Nodes for skeleton (not all nodes necessary are bones)
    // Create Copy of node hiearchy with only necessary nodes for animation
    // Copy nodes will have additional information( BoneID and Offset); If node is not bone then id == -1


SkeletonNode LoadSkeleton(std::string const& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("C:/Users/tj.albertson.C-P-U/source/repos/TJ-Albertson/game/resources/objects/vampire/dancing_vampire.dae", aiProcess_Triangulate);

    // Used in BoneCheck to find Node associated with Bone
    aiRootNode = scene->mRootNode;

    CreateBoneMap(scene->mRootNode, scene);

    BoneCheckRoot(scene->mRootNode, scene);

    SkeletonNode skeletonRootNode;

    CreateSkeleton(scene->mRootNode, skeletonRootNode);

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

        for (int i = 0; i < node->mNumMeshes; i++) {

            int meshIndex = node->mMeshes[i];

            for (int j = 0; j < scene->mMeshes[meshIndex]->mNumBones; j++) {
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

        for (int i = 0; i < node->mNumMeshes; i++) {

            int meshIndex = node->mMeshes[i];

            for (int j = 0; j < scene->mMeshes[meshIndex]->mNumBones; j++) {

                aiString boneNodeName = scene->mMeshes[meshIndex]->mBones[j]->mName;

                if (BoneMap.find(boneNodeName.C_Str()) == BoneMap.end()) {

                    aiMatrix4x4 offset = scene->mMeshes[meshIndex]->mBones[j]->mOffsetMatrix;

                    glm::mat4 glm_offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(offset);

                    Bone newBone = { BoneID++, glm_offset };

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

void FindSkeletonRoot();
// idea. same as bonecheck/bonecheckroot

// this is bad i think? after first skeleton node all following are bones
// Is the Skeleton ususally the child of the root node?
// Are there any children in the skelton hierachy that arent't bones?


void CreateSkeleton(const aiNode* node, SkeletonNode& skeletonNode)
{

	std::string nodeName = node->mName.C_Str();
	int index = -1;
    glm::mat4 offset;

	if (BoneMap.find(nodeName) != BoneMap.end()) {
		index = BoneMap[nodeName].ID;
        offset = BoneMap[nodeName].Offset;
	}

    skeletonNode.m_NodeName = node->mName.data;
    skeletonNode.m_NumChildren = node->mNumChildren;

    skeletonNode.id = index;
    skeletonNode.m_Offset = offset;

    skeletonNode.m_Transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(node->mTransformation);
   
	std::cout << skeletonNode.m_NodeName << " " << skeletonNode.id << std::endl;

	for (int i = 0; i < node->mNumChildren; i++) {

        SkeletonNode newBone;

		CreateSkeleton(node->mChildren[i], newBone);

        skeletonNode.m_Children.push_back(newBone);
	}
}

#endif