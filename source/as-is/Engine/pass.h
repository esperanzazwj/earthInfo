#pragma once
#include "RenderState.h"
#include <cereal/external/rapidjson/document.h>
#include "OpenglDriver/gl_program.h"
#include <map>
#include <unordered_map>
#include <set>
#include "engine_struct.h"
#include "VertexDataDesc.h"
#include "ResourceManager.h"
#include "PreDefine.h"
#include "UniformData.h"
#include "PerObjectUniform.h"
#include "RenderItem.h"
using namespace rapidjson;
using namespace HW;
using namespace std;

class Pass
{
public:
	Pass()
	{
		mRasterState = RasterState::getDefault();
		mBlendState = BlendState::getDefault();
		mDepthStencilState = DepthStencilState::getDefault();
		mClearState = ClearState::getDefault();
		mInputLayout = new VertexDataDesc;
	}
	void Render();

	Camera* camera;
	RenderQueue queue;
	RenderTarget* renderTarget;
	string name;
	as_Pass* as_pass;
	int bufferID = -1;

	// ----------------------------------------Render State-----------------------------------------
	RasterState 			mRasterState;
	BlendState  			mBlendState;
	DepthStencilState 		mDepthStencilState;
	ClearState				mClearState;

	bool	UseInstance = false;
	int InstanceBatchNum = 50;

	// ----------------------------------------Input Data-----------------------------------------
	//input uniform
	unordered_map<PerObjectUniform, string> mPerObjectUniform;
	unordered_map<string, UniformData> mPerFrameUniform;
	set<string>	mManualData;
	unordered_map<string, Sampler*>	mSamplerMap;
	VertexDataDesc*				mInputLayout;

	void setProgramConstantData(const string & name, Texture * val);

	void setProgramConstantData(const string & name, const void * val, string datatype, unsigned int size);

	void setSampler(const string& name, Sampler* s);
	Sampler* getSampler(const string& name);
};

class as_Pass
{
public:
	as_Pass()
	{
		name = "";
		mProgram = (NULL);
	}


	~as_Pass(){}

	string name;
	GpuProgram*			mProgram;

};

class PassManager :public ResourceManager<as_Pass*>{
public:
	static PassManager& getInstance()
	{
		static PassManager    instance;
		return instance;
	}
	
	Pass* LoadPass(const string& name, const string& path, const vector<string>& effects = {});
	void ParseAttributeFromJson(rapidjson::Value& attributes, VertexDataDesc* mInputLayout);
	void ParseUniformInputFromJson(rapidjson::Value& uniformmap, Pass* pass);
	Pass* ParsePassFromJsonFile(string filename, const vector<string>& effects = {});
	
	map<string, unsigned int> m_StringSemanticMap;
	
private:

private:	
	PassManager(){
	}
	PassManager(PassManager const&);              // Don't Implement.
	void operator=(PassManager const&); // Don't implement
};
