#pragma once
#include "engine_struct.h"

void createBonePos(as_Skeleton* skeleton, vector<float>& g_bonePos);
void createSRTmat(Quaternion& rotate, Vector3& position, Vector3& scale, Matrix4& SRT);

void getTimeSRT(double time, as_NodeAnimation* node, Matrix4& SRT);

void CalulateTransformationMatrices(as_SkeletonAnimation* animation, double tick, as_Skeleton* skeleton);
void CalcCombinedMatrix(as_Bone* frame, Matrix4 parentMatrix);


void calcFinalMatrices(as_Skeleton* skeleton, map<int, Matrix4>& Final_matrix);
//input: animation,skeleton,time	output:matrices for every bone of the skeleton
void updateSkeleton(as_SkeletonAnimation* ani, float time, as_Skeleton* ske, map<int, Matrix4>& Final_matrix);

