#pragma once
#include "mathlib.h"
#include <map>
#include <vector>
#include "PreDefine.h"
using namespace std;
using namespace HW;




struct	as_Light{
	Vector3  ColorAmbient;
	Vector3  ColorDiffuse;
	Vector3  ColorSpecular;
	Vector3  direction;
	Vector3  position;
	Vector3 attenuation;
	float MaxDistance;
	int  type;
};



struct as_Mesh{
	string name;
	as_Geometry* geometry;
	as_Material* material;
	Geometry*	 renderable = NULL;//暂时使用
	
	int materialID;//relevent id to the model file,should not use
};

struct as_Material{
	string name;
	Vector3 diffuse;
	Vector3 ambient;
	Vector3 specular;
	Vector3 emissive;
	Vector3 transparent;

	float roughness = 0;
	float metalness = 0;
	float opacity = 1;
	float shininess = 0;

	map<string, as_TexturePath> textures;

};


#define  MaxBonePerVertex 4
#define  MaxBoneIndices 65535

struct as_Geometry{
public:
	string name;
	int mNumVertices;
	int mNumFaces;
	int mNumBones;

	int has_skeleton = 0;

	vector<float> position;
	vector<float> normal;
	vector<float> texcoord;
	vector<float> tangent;
	vector<float> bitangent;
	vector<unsigned int> indices;

	vector<int> boneID;
	vector<float> boneWeight;
	vector<float> att_debug;
	as_Skeleton* mSkeleton;

};
struct as_Bone{

	string name;
	Matrix4 offsetMatrix;//bindPose下root space 转换到 bone space 
	Matrix4 mTramsformMatrix;//每一帧计算动画时，临时存放transform矩阵
	int id;
	as_Bone* parent;
	vector<as_Bone*> children;
	as_Bone(void){}
	as_Bone(string _name, Matrix4 _offsetMatrix, Matrix4 _mTramsformMatrix, int _id, as_Bone* _parent){
		name = _name;
		offsetMatrix = _offsetMatrix;
		mTramsformMatrix = _mTramsformMatrix;
		id = _id;
		parent = _parent;
	}
};

struct as_Skeleton{
	string name;
	as_Bone* root;
	vector<as_Bone*> bones;
	int num_bones;

	map<string, as_Bone*> bone_map;
};


struct vectorKey
{
	float time;
	Vector3 value;
};
struct quaterionKey{
	float time;
	Quaternion quat;//quaterion
};
struct as_NodeAnimation
{
	string name;//the bone name;
	int numPositionkeys;
	int numRotatekeys;
	int numScalekeys;
	vector<vectorKey> positionKeys;
	vector<quaterionKey> rotateKeys;
	vector<vectorKey> scaleKeys;
};
//only skeleton animation now
struct as_SkeletonAnimation{
	string name;
	float duration;
	float tickPerSecond;
	int numChannels;
	vector<as_NodeAnimation*> channels;
};

struct as_TexturePath{
	string name;
	string path;
	as_TexturePath() {
	}
	as_TexturePath(string _name){
		this->name = _name;
	}
};


