/*-------------------------------------------------------------------------------\
animation.h                                                                      

Functions associated with loading animations and animating the skeleton of a model

\-------------------------------------------------------------------------------*/ 

#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <bone_animation.h>
#include <skeleton.h>

struct Animation {
    char* m_Name;

    float m_Duration;
    int m_TicksPerSecond;

    unsigned int m_NumBoneAnimations;
    BoneAnimationChannel* m_BoneAnimations;
};

float m_CurrentTime;
float m_DeltaTime;

void AnimateModel(float dt, Animation animation, SkeletonNode* rootNode, glm::mat4* FinalBoneMatrix);
void CalculateNodeTransform(Animation animation, SkeletonNode* node, glm::mat4* FinalBoneMatrix, glm::mat4 parentTransform);
glm::mat4 FindBoneAndGetTransform(Animation animation, const char* boneNodeName, float animationTime);

Animation* LoadAnimations(unsigned int mNumAnimations, aiAnimation** mAnimations);
BoneAnimationChannel* LoadBoneAnimationChannels(unsigned int mNumChannels, aiNodeAnim** mChannels);

glm::mat4 FindBoneAndGetTransform(Animation animation, const char* boneNodeName, float animationTime)
{
    for (int i = 0; i < animation.m_NumBoneAnimations; ++i) {

        if (std::strcmp(animation.m_BoneAnimations[i].m_NodeName, boneNodeName) == 0) {
            return getBoneAnimationTransformation(&animation.m_BoneAnimations[i], animationTime);
        }
    }
}

void CalculateNodeTransform(Animation animation, SkeletonNode* node, glm::mat4* FinalBoneMatrix, glm::mat4 parentTransform)
{
    glm::mat4 nodeTransform = node->m_Transformation;

    bool isBoneNode = (node->id >= 0);
    if (isBoneNode) {
        nodeTransform = FindBoneAndGetTransform(animation, node->m_NodeName, m_CurrentTime);
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    if (isBoneNode) {
        glm::mat4 finalBoneMatrix = globalTransformation * node->m_Offset;
        FinalBoneMatrix[node->id] = finalBoneMatrix;
    }

    for (int i = 0; i < node->m_NumChildren; ++i) {
        CalculateNodeTransform(animation, node->m_Children[i], FinalBoneMatrix, globalTransformation);
    }
}

void AnimateModel(float dt, Animation animation, SkeletonNode* rootNode, glm::mat4* FinalBoneMatrix)
{
    m_DeltaTime = dt;
    m_CurrentTime += animation.m_TicksPerSecond * dt;
    m_CurrentTime = fmod(m_CurrentTime, animation.m_Duration);

    CalculateNodeTransform(animation, rootNode, FinalBoneMatrix, glm::mat4(1.0f));
}


Animation* LoadAnimations(unsigned int mNumAnimations, aiAnimation** mAnimations) {

    Animation* m_Animations = (Animation *)malloc(mNumAnimations * sizeof(Animation));

    for (int i = 0; i < mNumAnimations; ++i) {

        aiAnimation* aiAnimation = mAnimations[i];

        const char* mName  = aiAnimation->mName.C_Str();
        size_t mNameLength = aiAnimation->mName.length;

        m_Animations[i].m_Name = (char*)malloc(mNameLength * sizeof(char));
        std::strcpy(m_Animations[i].m_Name, mName);

        m_Animations[i].m_Duration = aiAnimation->mDuration;
        m_Animations[i].m_TicksPerSecond = aiAnimation->mTicksPerSecond;
        m_Animations[i].m_NumBoneAnimations = aiAnimation->mNumChannels;

        m_Animations[i].m_BoneAnimations = LoadBoneAnimationChannels(aiAnimation->mNumChannels, aiAnimation->mChannels);
    }

    return m_Animations;
}

BoneAnimationChannel* LoadBoneAnimationChannels(unsigned int mNumChannels, aiNodeAnim** mChannels) {

    BoneAnimationChannel* m_BoneAnimations = (BoneAnimationChannel *)malloc(mNumChannels * sizeof(BoneAnimationChannel));

    for (int i = 0; i < mNumChannels; ++i) {

        aiNodeAnim* aiNodeAnim = mChannels[i];

        int m_NumPositions = aiNodeAnim->mNumPositionKeys;
        int m_NumRotations = aiNodeAnim->mNumRotationKeys;
        int m_NumScalings = aiNodeAnim->mNumScalingKeys;

        const char* nodemName = aiNodeAnim->mNodeName.C_Str();
        size_t nameLength     = aiNodeAnim->mNodeName.length;

        m_BoneAnimations[i].m_NodeName = (char*)malloc(nameLength * sizeof(char));
        std::strcpy(m_BoneAnimations[i].m_NodeName, nodemName);

        m_BoneAnimations[i].m_NumPositions = m_NumPositions;
        m_BoneAnimations[i].m_NumRotations = m_NumRotations;
        m_BoneAnimations[i].m_NumScalings = m_NumScalings;

        KeyPosition* m_Positions = (KeyPosition*)malloc(m_NumPositions * sizeof(KeyPosition));
        KeyRotation* m_Rotations = (KeyRotation*)malloc(m_NumRotations * sizeof(KeyRotation));
        KeyScale*    m_Scales    = (KeyScale*)malloc(m_NumScalings * sizeof(KeyScale));

        for (int j = 0; j < m_NumPositions; ++j) {

            aiVector3D aiPosition = aiNodeAnim->mPositionKeys[j].mValue;

            float timeStamp = aiNodeAnim->mPositionKeys[j].mTime;

            glm::vec3 glm_position = AssimpGLMHelpers::GetGLMVec(aiPosition);

            m_Positions[j].position = glm_position;
            m_Positions[j].timeStamp = timeStamp;
        }
        m_BoneAnimations[i].m_Positions = m_Positions;

        for (int j = 0; j < m_NumRotations; ++j) {

            aiQuaternion aiOrientation = aiNodeAnim->mRotationKeys[j].mValue;

            float timeStamp = aiNodeAnim->mRotationKeys[j].mTime;

            glm::quat glm_quat = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
            
            m_Rotations[j].orientation = glm_quat;
            m_Rotations[j].timeStamp = timeStamp;
        }
        m_BoneAnimations[i].m_Rotations = m_Rotations;

        for (int j = 0; j < m_NumScalings; ++j) {
            
            aiVector3D scale = aiNodeAnim->mScalingKeys[j].mValue;
            
            float timeStamp = aiNodeAnim->mScalingKeys[j].mTime;

            glm::vec3 glm_scale = AssimpGLMHelpers::GetGLMVec(scale);
            
            m_Scales[j].scale = glm_scale;   
            m_Scales[j].timeStamp = timeStamp;
        }
        m_BoneAnimations[i].m_Scales = m_Scales;
    }
    return m_BoneAnimations;
}

#endif