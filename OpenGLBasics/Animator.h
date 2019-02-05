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
	float duration; // duration in ticks
	float ticksPerSec; // speed of the animation
	std::vector<Pose> poses;

	Animation() = default;
};


//###################################################################
//				BONE, POSE AND ANIMATION
//###################################################################

void searchPoses(const Animation& anim, Pose& pose1, Pose& pose2, const float currentTime)
{
	pose1 = anim.poses[0];
	if (!anim.poses.size())
		exit(264628466);

	if ( (anim.poses.size() == 1) || !anim.duration)
	{
		pose2 = pose1;
		return;
	}

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

void animate(const float currentTime, Animation& animation, std::vector<MUDLoader::tuple<int, glm::mat4*, glm::mat4*>>& bindTransforms, std::vector<glm::mat4>& currentTransforms)
{
	
	auto numberOfBones = bindTransforms.size();
	//bonePose relative to the parentBone
	std::vector<glm::mat4> bonePoseRelative(numberOfBones);
	//bonePose relative to the model
	std::vector<glm::mat4> bonePoseAbsolute(numberOfBones);
	
	currentTransforms.reserve(numberOfBones);

	// factor of the blending [0 : 1]
	const float factor = (currentTime * animation.ticksPerSec) / animation.duration;
	
	// poses are asumed to be the same size (size = meshBonesCount)
	Pose pose1, pose2;
	searchPoses(animation, pose1, pose2, currentTime);

	// APPLY animation
	for (size_t i = 0; i < numberOfBones; i++)
	{
		// this is the animation
		glm::mat4 transform = interpolatePoseTransform(pose1.transforms[i], pose2.transforms[i], factor);
		// apply it to the bone
		bonePoseRelative[i] = (*bindTransforms[0].first) * transform;
	}

	// TRANSFORM parent space
	bonePoseAbsolute[0] = bonePoseRelative[0];
	for (size_t i = 1; i < numberOfBones; i++)
	{
		int parentID = bindTransforms[i].parentID;
		bonePoseAbsolute[i] = bonePoseAbsolute[parentID] * bonePoseRelative[i];
	}

	// APPLY modelTransform
	for (size_t i = 0; i < numberOfBones; i++)
	{
		currentTransforms[i] = bonePoseAbsolute[i] * (*bindTransforms[i].second);
	}
};
