#pragma once
#include <string>
#include <vector>
#include "PreDefine.h"
using namespace HW;
using namespace std;


enum UniformType
{
	UT_Value,
	UT_Texture
};

struct UniformData {
	vector<uint8_t> data;
	Texture* tex=NULL;
	string datatype;
	int size=0;
	UniformType type;

	static int GetTypeSize(const string& type) {
		if (type == "mat4")
		{
			return 64;
		}
		if (type == "mat3")
		{
			return 36;
		}
		if (type == "vec4")
		{
			return 16;
		}
		if (type == "ivec4")
		{
			return 16;
		}
		if (type == "vec3")
		{
			return 12;
		}
		if (type == "ivec3")
		{
			return 12;
		}
		if (type == "vec2")
		{
			return 8;
		}
		if (type == "ivec2")
		{
			return 8;
		}
		if (type == "float")
		{
			return 4;
		}
		if (type == "int")
		{
			return 4;
		}
		if (type == "uint")
		{
			return 4;
		}
		throw std::runtime_error{ "Invalid type" };
	}
};