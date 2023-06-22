#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>


#include <BoneAnimationChannel.h>

struct Animation {
    std::string m_Name;

    float m_Duration;
    int m_TicksPerSecond;

    unsigned int m_NumBoneAnimations;
    BoneAnimationChannel* m_BoneAnimations;
};

BoneAnimationChannel* LoadBoneAnimationChannels(unsigned int mNumChannels, aiNodeAnim** mChannels);

glm::mat4 FindBoneAndGetTransform(Animation animation, std::string boneNodeName, float animationTime)
{
    for (int i = 0; i < animation.m_NumBoneAnimations; i++) {
        if (animation.m_BoneAnimations[i].m_NodeName == boneNodeName) {
            return getBoneAnimationTransformation(&animation.m_BoneAnimations[i], animationTime);
        }
    }
}

Animation* LoadAnimations(unsigned int mNumAnimations, aiAnimation** mAnimations) {

    //Animation* m_Animations = new Animation[mNumAnimations];

    Animation* m_Animations = (Animation *)malloc(mNumAnimations * sizeof(Animation));

    if (m_Animations == NULL) {
        std::cout << "m_Animations Alloc failed" << std::endl;
    }

    for (int i = 0; i < mNumAnimations; ++i) {

        aiAnimation* aiAnimation = mAnimations[i];

        m_Animations[i].m_Name = aiAnimation->mName.C_Str();
        m_Animations[i].m_Duration = aiAnimation->mDuration;
        m_Animations[i].m_TicksPerSecond = aiAnimation->mTicksPerSecond;
        m_Animations[i].m_NumBoneAnimations = aiAnimation->mNumChannels;

        m_Animations[i].m_BoneAnimations = LoadBoneAnimationChannels(aiAnimation->mNumChannels, aiAnimation->mChannels);
    }

    return m_Animations;
}

BoneAnimationChannel* LoadBoneAnimationChannels(unsigned int mNumChannels, aiNodeAnim** mChannels) {

    //BoneAnimationChannel* boneAnimationChannel = new BoneAnimationChannel[mNumChannels];

    BoneAnimationChannel* boneAnimationChannel = (BoneAnimationChannel *)malloc(mNumChannels * sizeof(BoneAnimationChannel));

    for (int i = 0; i < mNumChannels; i++) {

        aiNodeAnim* aiNodeAnim = mChannels[i];

        int m_NumPositions = aiNodeAnim->mNumPositionKeys;
        int m_NumRotations = aiNodeAnim->mNumRotationKeys;
        int m_NumScalings = aiNodeAnim->mNumScalingKeys;

        boneAnimationChannel[i].m_NodeName = aiNodeAnim->mNodeName.C_Str();

        boneAnimationChannel[i].m_NumPositions = m_NumPositions;
        boneAnimationChannel[i].m_NumRotations = m_NumRotations;
        boneAnimationChannel[i].m_NumScalings = m_NumScalings;

        KeyPosition* m_Positions = (KeyPosition*)malloc(m_NumPositions * sizeof(KeyPosition));
        KeyRotation* m_Rotations = (KeyRotation*)malloc(m_NumRotations * sizeof(KeyRotation));
        KeyScale*    m_Scales    = (KeyScale*)malloc(m_NumScalings * sizeof(KeyScale));

        for (int j = 0; j < m_NumPositions; ++j) {

            aiVector3D aiPosition = aiNodeAnim->mPositionKeys[j].mValue;

            float timeStamp = aiNodeAnim->mPositionKeys[j].mTime;

            m_Positions[j].position = AssimpGLMHelpers::GetGLMVec(aiPosition);
            m_Positions[j].timeStamp = timeStamp;
        }
        boneAnimationChannel[i].m_Positions = m_Positions;

        for (int j = 0; j < m_NumRotations; ++j) {

            aiQuaternion aiOrientation = aiNodeAnim->mRotationKeys[j].mValue;

            float timeStamp = aiNodeAnim->mRotationKeys[j].mTime;
            
            m_Rotations[j].orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
            m_Rotations[j].timeStamp = timeStamp;
        }
        boneAnimationChannel[i].m_Rotations = m_Rotations;

        for (int j = 0; j < m_NumScalings; ++j) {
            
            aiVector3D scale = aiNodeAnim->mScalingKeys[j].mValue;
            
            float timeStamp = aiNodeAnim->mScalingKeys[j].mTime;
            
            m_Scales[j].scale = AssimpGLMHelpers::GetGLMVec(scale);   
            m_Scales[j].timeStamp = timeStamp;
        }
        boneAnimationChannel[i].m_Scales = m_Scales;

        return boneAnimationChannel;
    }
}

#endif