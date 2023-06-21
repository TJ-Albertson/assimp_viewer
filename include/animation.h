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

        newBoneChannel.m_NodeName = aiNodeAnim->mNodeName;
        new
        newBoneChannel.m_Positions = aiNodeAnim->mPositionKeys;
        
    }
}

#endif