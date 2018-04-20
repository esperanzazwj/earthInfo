#pragma once
#include "Pipeline/PassGraph.h"
#include "pass.h"
#include <vector>
#include <string>
#include <map>
using namespace std;

#define EffectDecl(c) \
	c() { this->name = #c; } \
	void Init() override; \
	void Update() override; \
	void GetInputPasses(vector<Pass*>&, vector<pair<Pass*, Pass*>>&, vector<pair<Texture*&, Texture*&>>&) override;

class PassGraph;
class PipelineEffect {

public:
	virtual void Init() 
	{
	}

	/**
	** deprecated. render call is moved to Pass, and called by pass graph
	*/
	virtual void Render() {};

	/**
	** update operation. update resources before render. Uniform, Texture, etc.
	*/
	virtual void Update() {};

	/**
	** post process.
	*/
	virtual void PostProcess() {};

	/**
	** provide passes & passes relationship. called by pass graph.
	*/
	virtual void GetInputPasses(vector<Pass*>&, vector<pair<Pass*, Pass*>>&, vector<pair<Texture*&, Texture*&>>&) {};

	/**
	** update texture to merge pass
	*/
	virtual void UpdateToMergePass(Pass*) {};

	/**
	** get first pass of effect
	*/
	virtual Pass* Head() { return NULL; };

	/**
	** get last pass of effect
	*/
	virtual Pass* Tail() { return NULL; };

	/**
	** unique string
	*/
	string name;
	double time;


	/**
	** unique mask. different effect set specific bit to '1'
	*/
	unsigned int mask;

	/**
	** render queue. read from scene config file during init.
	*/
	RenderQueue queue;

};
