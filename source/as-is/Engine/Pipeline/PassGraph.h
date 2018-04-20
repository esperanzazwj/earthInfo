#pragma once
#include <map>
#include <unordered_map>
#include <set>
#include <string>
#include "../pass.h"
#include "../effect.h"

using namespace HW;
class PipelineEffect;
class GraphNode
{

public :
	GraphNode() {}
	GraphNode(std::string name) :name(name){}
	std::string name;
	int inDegree = 0, outDegree = 0, curInDegree = 0, curOutDegree = 0;
	GraphNode* next = NULL; // point to all children
	GraphNode* pre = NULL; // point to all ancestors
};

class PassGraph
{
public:
	void Init();
	void Update();
	void Render();
	void PostProcess();
	void Render(Pass* pass);
	void SetInput(Pass const* sourcePass, Pass const* targetPass, Texture*& sourceTexture, Texture*& tergetTexture);
	void SetInputPass(Pass const* sourcePass, Pass const* targetPass);
	void SetInputTexture(Texture*& sourceTexture, Texture*& tergetTexture);
	void DeleteInput(Pass const* sourcePass, Pass const* targetPass);
	void AttachPass(Pass* pass);
	void PushEffect(PipelineEffect* effect);
	void AttachEffect(PipelineEffect* effect);
	void AttachAllEffects();
	void DetachEffect(PipelineEffect* effect);

	/**
	**	merge to passes so that they share the same render target, and return the pass who owns the RT
	*/
	Pass* MergePass(Pass* originPass, Pass* targetPass);

	/**
	**	collect objects into coresponding effects
	*/
	void CollectObject(const SceneManager*);

	/**
	**	Helper functions, to print the graph
	*/
	void PrintGraph();

	double time;
private:
	void RestoreState();
	std::unordered_map<std::string, GraphNode* > passGraph;
	std::unordered_map<std::string, Pass*> passes;
	std::unordered_map<std::string, PipelineEffect*> effects;
	std::vector<int> hashMergeBuffer;
};
