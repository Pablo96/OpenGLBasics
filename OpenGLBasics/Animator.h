#pragma once

//###################################################################
//				BONE, POSE AND ANIMATION
//###################################################################
#define MAX_VERTEX_BONES 4	// Max number of bones per vertex

struct Bone
{
	int id;
	Bone* parent;
	glm::mat4 bindOffset;	// Original offset relative to the parent
	glm::mat4 inverseBindOffset;
	std::vector<Bone*> children;
	const char* debugName;
};

// TODO: add scaling
// NOTE: this are relative to parent bone.
struct PoseKey
{
	glm::vec3 position;
	glm::quat rotation;
};

struct Pose
{
	float timeStamp; // time is played (in seconds)
	std::vector<PoseKey> transforms;
};

struct Animation
{
	const float duration; // duration in ticks
	float ticksPerSec; // speed of the animation
	std::vector<Pose> poses;
};


//###################################################################
//				BONE, POSE AND ANIMATION
//###################################################################

void searchPoses(const Animation& anim, Pose& pose1, Pose& pose2, const float currentTime)
{
	pose1 = anim.poses[0];
	for (size_t i = 1; i < anim.poses.size(); i++)
	{
		if (anim.poses[i].timeStamp > currentTime)
		{
			pose2 = anim.poses[i];
			pose1 = anim.poses[i - 1];
			return;
		}
	}
}

glm::mat4 interpolatePoseTransform(const PoseKey& trans1, const PoseKey& trans2, const float factor)
{
	glm::quat slerpQuat = glm::slerp(trans1.rotation, trans2.rotation, factor);
	glm::mat4 rotMat = glm::toMat4(slerpQuat);

	glm::vec3 lerpVec = glm::mix(trans1.position, trans2.position, factor);
	glm::mat4 posMat = glm::translate(lerpVec);

	return posMat * rotMat;
}

void animate(const float currentTime, Animation& animation, std::vector<glm::mat4>& currentTransforms)
{
	// factor of the blending [0 : 1]
	const float factor = (currentTime * animation.ticksPerSec) / animation.duration;
	
	// poses are asumed to be the same size (size = meshBonesCount)
	Pose pose1, pose2;
	searchPoses(animation, pose1, pose2, currentTime);


	for (size_t i = 0; i < pose1.transforms.size(); i++)
	{
		glm::mat4 transform = interpolatePoseTransform(pose1.transforms[i], pose2.transforms[i], factor);
		currentTransforms.emplace_back(transform);
	}
};
