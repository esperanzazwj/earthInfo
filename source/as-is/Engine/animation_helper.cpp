#include "animation_helper.h"

void createBonePos(as_Skeleton* skeleton, vector<float>& g_bonePos)
{

	for (auto& bone : skeleton->bones) {
		//Matrix4 offset_mat = (bone->offsetMatrix);
		Matrix4 transform_mat = bone->mTramsformMatrix;
		Vector4 posP = (transform_mat)*Vector4(0, 0, 0, 1);
		Vector4 parentPos = Vector4(0, 0, 0, 1);
		if (bone->parent != NULL) {
			Matrix4 offset_mat_parent = bone->parent->mTramsformMatrix;
			parentPos = (offset_mat_parent)*Vector4(0, 0, 0, 1);
		}
		g_bonePos.push_back(posP.x);
		g_bonePos.push_back(posP.y);
		g_bonePos.push_back(posP.z);
		g_bonePos.push_back(posP.w);

		g_bonePos.push_back(parentPos.x);
		g_bonePos.push_back(parentPos.y);
		g_bonePos.push_back(parentPos.z);
		g_bonePos.push_back(parentPos.w);
	}
}

void createSRTmat(Quaternion& rotate, Vector3& position, Vector3& scale, Matrix4& SRT)
{
    SRT = Matrix4{rotate, scale, position};
}

void getTimeSRT(double time, as_NodeAnimation* node, Matrix4& SRT)
{
	int posIndex = 0;
	while (true) {
		if (posIndex + 1 >= node->numPositionkeys) break;
		if (node->positionKeys[posIndex + 1].time > time)	break;
		posIndex++;
	}
	int RotateIndex = 0;
	while (true) {
		if (RotateIndex + 1 >= node->numRotatekeys) break;
		if (node->rotateKeys[RotateIndex + 1].time > time)	break;
		RotateIndex++;
	}
	int ScaleIndex = 0;
	while (true) {
		if (ScaleIndex + 1 >= node->numScalekeys) break;
		if (node->scaleKeys[ScaleIndex + 1].time > time)	break;
		ScaleIndex++;
	}
	createSRTmat(node->rotateKeys[RotateIndex].quat, node->positionKeys[posIndex].value,
		node->scaleKeys[posIndex].value, SRT);
}

void CalulateTransformationMatrices(as_SkeletonAnimation* animation, double tick, as_Skeleton* skeleton)
{
	while (tick > animation->duration)
		tick -= animation->duration;
	for (int i = 0; i < animation->numChannels; i++) {
		auto ch = animation->channels[i];
		Matrix4 SRT;
		getTimeSRT(tick, ch, SRT);
		skeleton->bone_map[ch->name]->mTramsformMatrix = SRT;
	}
}

void CalcCombinedMatrix(as_Bone* frame, Matrix4 parentMatrix)
{
	frame->mTramsformMatrix = parentMatrix*frame->mTramsformMatrix;
	for (auto& child : frame->children)
		CalcCombinedMatrix(child, frame->mTramsformMatrix);
}

void calcFinalMatrices(as_Skeleton* skeleton, map<int, Matrix4>& Final_matrix)
{
	for (int i = 0; i < (int)skeleton->bones.size(); i++) {
		as_Bone* b = skeleton->bones[i];
		Final_matrix[i] = b->mTramsformMatrix*b->offsetMatrix;
	}
}

void updateSkeleton(as_SkeletonAnimation* ani, float time, as_Skeleton* ske, map<int, Matrix4>& Final_matrix)
{
	CalulateTransformationMatrices(ani, time, ske);
	CalcCombinedMatrix(ske->root, {});
	calcFinalMatrices(ske, Final_matrix);
}

