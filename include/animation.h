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

glm::mat4 FindBoneAndGetTransform(Animation* animation, std::string boneNodeName, float animationTime)
{
    for (int i = 0; i < animation->m_NumBoneAnimations; i++) {
        if (animation->m_BoneAnimations[i].m_NodeName == boneNodeName) {
            return getBoneAnimationTransformation(&animation->m_BoneAnimations[i], animationTime);
        }
    }
}

Animation* LoadAnimations(unsigned int mNumAnimations, aiAnimation** mAnimations) {

    Animation* m_Animations = new Animation[mNumAnimations];

    for (int i = 0; i < mNumAnimations; i++) {

        aiAnimation* aiAnimation = mAnimations[i];

        Animation newAnimation;

        newAnimation.m_Name = aiAnimation->mName.C_Str();
        newAnimation.m_Duration = aiAnimation->mDuration;
        newAnimation.m_TicksPerSecond = aiAnimation->mTicksPerSecond;
        newAnimation.m_NumBoneAnimations = aiAnimation->mNumChannels;

        newAnimation.m_BoneAnimations = LoadBoneAnimationChannels(aiAnimation->mNumChannels, aiAnimation->mChannels);

        m_Animations[i] = newAnimation;
    }
}

BoneAnimationChannel* LoadBoneAnimationChannels(unsigned int mNumChannels, aiNodeAnim** mChannels) {

    BoneAnimationChannel* boneAnimationChannel = new BoneAnimationChannel[mNumChannels];

    for (int i = 0; i < mNumChannels; i++) {

        aiNodeAnim* aiNodeAnim = mChannels[i];

        BoneAnimationChannel newBoneChannel;

        newBoneChannel.m_NodeName = aiNodeAnim->mNodeName.C_Str();
        newBoneChannel.m_NumPositions = aiNodeAnim->mNumPositionKeys;
        newBoneChannel.m_NumRotations = aiNodeAnim->mNumRotationKeys;
        newBoneChannel.m_NumScalings = aiNodeAnim->mNumScalingKeys;

        KeyPosition* keyPositions = new KeyPosition[aiNodeAnim->mNumPositionKeys];
        KeyRotation* keyRotations = new KeyRotation[aiNodeAnim->mNumRotationKeys];
        KeyScale* keyScales = new KeyScale[aiNodeAnim->mNumScalingKeys];

        for (int i = 0; i < aiNodeAnim->mNumPositionKeys; ++i) {

            aiVector3D aiPosition = aiNodeAnim->mPositionKeys[i].mValue;

            float timeStamp = aiNodeAnim->mPositionKeys[i].mTime;

            KeyPosition data;

            data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);

            data.timeStamp = timeStamp;

            keyPositions[i] = data;
        }

        newBoneChannel.m_Positions = keyPositions;

        for (int i = 0; i < aiNodeAnim->mNumRotationKeys; ++i) {

            aiQuaternion aiOrientation = aiNodeAnim->mRotationKeys[i].mValue;

            float timeStamp = aiNodeAnim->mRotationKeys[i].mTime;
            
            KeyRotation data;
            
            data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
            
            data.timeStamp = timeStamp;
            
            keyRotations[i] = data;
        }

        newBoneChannel.m_Rotations = keyRotations;

        for (int keyIndex = 0; keyIndex < aiNodeAnim->mNumScalingKeys; ++keyIndex) {
            
            aiVector3D scale = aiNodeAnim->mScalingKeys[keyIndex].mValue;
            
            float timeStamp = aiNodeAnim->mScalingKeys[keyIndex].mTime;
            
            KeyScale data;
            
            data.scale = AssimpGLMHelpers::GetGLMVec(scale);
            
            data.timeStamp = timeStamp;
            
            keyScales[i] = data;
        }

        newBoneChannel.m_Scales = keyScales;

        return &newBoneChannel;
    }
}

#endif