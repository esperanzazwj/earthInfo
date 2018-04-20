#pragma once
#include <vector>
#include <stdexcept>
using namespace std;

struct  VertexElement
{
	string semantic;
	string name;
	int index;
	string type;
};

class VertexDataDesc{
public:

	vector<VertexElement*> getElements(){
		return mAttributes;
	}
	void clear(){
		mAttributes.clear();
	}
	void add(VertexElement* desc){
		mAttributes.push_back(desc);
	}

	void getAttributeDataType(VertexElement* att, string& datatype, int& size)
	{
		if (att->type == "vec2"){
			size = 2;
			datatype = "float";
			return;
		}
		if (att->type == "vec3"){
			size = 3;
			datatype = "float";
			return;
		}
		if (att->type == "vec4"){
			size = 4;
			datatype = "float";
			return;
		}
		if (att->type == "mat4") {
			size = 16;
			datatype = "float";
			return;
		}
		if (att->type == "ivec4"){
			size = 4;
			datatype = "int";
			return;
		}
		throw std::runtime_error("Invalid type: " + att->type);
	}
private:
	vector<VertexElement*> mAttributes;
};