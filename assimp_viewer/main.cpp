#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

std::vector<std::string> BoneNames;

void GetBoneNames(aiScene scene);

int main()
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("C:/Users/tj.albertson.C-P-U/source/repos/TJ-Albertson/game/resources/objects/vampire/dancing_vampire.dae", aiProcess_Triangulate);

    GetBoneNames(*scene);

	//std::cout << "Hello World" << std::endl;




	return 1;
}

void GetBoneNames(const aiScene scene) {
    for (int i = 0; i < scene.mNumMeshes; i++) {

        if (scene.mMeshes[i]->mNumBones == 0) continue;

        for (int j = 0; j < scene.mMeshes[i]->mNumBones; j++) {

            BoneNames.push_back(scene.mMeshes[i]->mBones[j]->mName.C_Str());
            std::cout << scene.mMeshes[i]->mBones[j]->mName.C_Str() << std::endl;
        }
    }
}

void CopyNodesWithMeshes(aiNode node, SceneObject targetParent, Matrix4x4 accTransform)
{

    // if node has meshes, create a new scene object for it
    if (node.mNumMeshes > 0)
    {

    }
    else
    {

    }
    // continue for all child nodes
    for (all node.mChildren)
        CopyNodesWithMeshes(node.mChildren[a], parent, transform);
}