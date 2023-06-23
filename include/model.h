#ifndef MODEL_H
#define MODEL_H

#define MAX_BONE_INFLUENCE 4

#include <glad/glad.h> // holds all OpenGL type declarations

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <stb_image.h>

#include <fstream>
#include <iostream>
#include <assimp_glm_helpers.h>
#include <string>
#include <vector>


#include <animate_function.h>
#include <skeleton.h>

struct VertexData {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

struct Mesh {
	unsigned int* VAO;

	int numIndices;
	unsigned int* indices;
	
	int numTextures;
	Texture* textures;
};

struct Model {
	const char* m_Name;

	int m_NumMeshes;
	Mesh* m_Meshes;

	int m_NumAnimations;
	Animation* m_Animations;

	glm::mat4 m_FinalBoneMatrices[100];
	SkeletonNode* rootSkeletonNode;
};


std::string directory;
// Need to add ID's/ change to index for final bone array

Model* LoadModel(std::string const& path);
void processNode(aiNode* node, const aiScene* scene, Model* model);
Mesh processMesh(aiMesh* mesh, const aiScene* scene);
std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
void AssignBoneId(std::vector<VertexData>& vertexData, aiMesh* mesh, const aiScene* scene);
unsigned int* LoadMeshVertexData(std::vector<VertexData> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

Model* LoadModel(std::string const& path) {

	

	

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

	Model* newModel = (Model*)malloc(sizeof(Model));
	
	newModel->m_Name = "Vampire";

	newModel->m_NumMeshes = scene->mNumMeshes;
	newModel->m_Meshes = (Mesh*)malloc(scene->mNumMeshes * sizeof(Mesh));

	newModel->m_NumAnimations = scene->mNumAnimations;
	newModel->m_Animations = LoadAnimations(scene->mNumAnimations, scene->mAnimations);

	newModel->rootSkeletonNode = LoadSkeleton(path);

	for (int i = 0; i < 100; i++) {
		newModel->m_FinalBoneMatrices[i] = glm::mat4(1.0f);
	}

	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene, newModel);

	return newModel;
}

void processNode(aiNode* node, const aiScene* scene, Model* model) {

	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		model->m_Meshes[i] = processMesh(mesh, scene);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene, model);
	}
}

Mesh processMesh(aiMesh* mesh, const aiScene* scene) {

	int numVertices = mesh->mNumVertices;
	int numFaces = mesh->mNumFaces;
	
	int numIndices = 0;
	for (int i = 0; i < numFaces; ++i)
		numIndices += mesh->mFaces[i].mNumIndices;




	VertexData* vertices = (VertexData*)malloc(numVertices * sizeof(VertexData));
	unsigned int* indices = (unsigned int *)malloc(numIndices * sizeof(unsigned int));

	std::vector<Texture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		VertexData vertexData;
		vertexData.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
		vertexData.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

		if (mesh->mTextureCoords[0]) {
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertexData.TexCoords = vec;
		}
		else {
			vertexData.TexCoords = glm::vec2(0.0f, 0.0f);
		}
			
		vertices[i] = vertexData;
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	int numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);



	std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

	std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

	std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	AssignBoneId(vertices, mesh, scene);

	unsigned int* VAO = LoadMeshVertexData(vertices, indices, textures);

	Mesh newMesh = { VAO, indices, textures };

	return newMesh;
}


void AssignBoneId(std::vector<VertexData>& vertexData, aiMesh* mesh, const aiScene* scene) {

	int vertexId;

	int boneId;
	float weight;

	for (int j = 0; j < mesh->mNumBones; ++j) {

		unsigned int boneID = 0;

		std::string boneName = mesh->mBones[j]->mName.C_Str();


		if (BoneMap[boneName].ID) {
			boneID = BoneMap[boneName].ID;
		}

		int numWeights = mesh->mBones[j]->mNumWeights;

		for (int k = 0; k < numWeights; ++k) {

			int vertexId = mesh->mBones[j]->mWeights[k].mVertexId;
			float weight = mesh->mBones[j]->mWeights[k].mWeight;

			for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
				if (vertexData[vertexId].m_BoneIDs[i] < 0) {

					vertexData[vertexId].m_Weights[i] = weight;
					vertexData[vertexId].m_BoneIDs[i] = boneID;
					break;
				}
			}
		}
	}
}


unsigned int* LoadMeshVertexData(std::vector<VertexData> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
	unsigned int VAO, VBO, EBO;

	// initializes all the buffer objects/arrays
	// now that we have all the required data, set the vertex buffers and its attribute pointers.
	// create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	// load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// A great thing about structs is that their memory layout is sequential for all its items.
	// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
	// again translates to 3/2 floats which translates to a byte array.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, TexCoords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, Tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, Bitangent));
	// ids
	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(VertexData), (void*)offsetof(VertexData, m_BoneIDs));

	// weights
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, m_Weights));
	glBindVertexArray(0);

	return &VAO;
}


unsigned int TextureFromFile(const char* path, const std::string& directory)
{
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.

std::vector<Texture> textures_loaded;
Texture* loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	int numTextures = mat->GetTextureCount(type);

	Texture* textures = (Texture*)malloc(numTextures * sizeof(Texture));

	for (unsigned int i = 0; i < numTextures; i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++) {
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
				textures[i] = (textures_loaded[j]);
				skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}

		if (!skip) { // if texture hasn't been loaded already, load it
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures[i] = texture;
			textures_loaded.push_back(texture); // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
		}
	}
	return textures;
}


void DrawModel(Model* model, unsigned int shaderID)
{
	for (unsigned int i = 0; i < model->m_NumMeshes; i++) {
		// bind appropriate textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;

		Mesh mesh = model->m_Meshes[i];

		for (unsigned int i = 0; i < mesh.textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
			// retrieve texture number (the N in diffuse_textureN)
			std::string number;
			std::string name = mesh.textures[i].type;
			if (name == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if (name == "texture_specular")
				number = std::to_string(specularNr++); // transfer unsigned int to string
			else if (name == "texture_normal")
				number = std::to_string(normalNr++); // transfer unsigned int to string
			else if (name == "texture_height")
				number = std::to_string(heightNr++); // transfer unsigned int to string

			// now set the sampler to the correct texture unit
			glUniform1i(glGetUniformLocation(shaderID, (name + number).c_str()), i);
			// and finally bind the texture
			glBindTexture(GL_TEXTURE_2D, mesh.textures[i].id);
		}

		// draw mesh
		glBindVertexArray(*mesh.VAO);
		glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
	}
}

#endif