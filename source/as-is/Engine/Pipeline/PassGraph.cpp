#include "PassGraph.h"
#include "../pass.h"
#include <iostream>
#include <queue>
#include <stack>
#include <algorithm>
#include "../SceneManager.h"
using std::queue;
using std::stack;

void PassGraph::Init() // resource init, effects set inner passes relationship
{
	for (auto effect : effects)
	{
		effect.second->Init();
		//effect.second->AttachPassesToGraph(this);
	}
}

void PassGraph::Update()
{
	for (auto effect : effects) // update constants or texture
	{
		effect.second->time = this->time;
		effect.second->Update();
	}

}


void PassGraph::PostProcess()
{
	for (auto effect : effects) 
	{
		effect.second->PostProcess();
	}
}

void PassGraph::Render() // topological sorting and render
{
	queue<GraphNode*> que;
	for (auto node : passGraph)
		if (node.second->curInDegree == 0)
			que.push(node.second);
	while (!que.empty())
	{
		GraphNode* frontNode = que.front();
		assert(frontNode->curInDegree == 0 && "que front in degree not zero!");
		auto p = passes[frontNode->name];
		if (p->bufferID >= 0 && hashMergeBuffer[p->bufferID] == 0) // first pass on this buffer
		{
			hashMergeBuffer[p->bufferID] = 1;
		}
		else if(p->bufferID >= 0)
			p->mClearState.clearFlag = false;

		p->Render();
		//cout << p->name << endl;
		for (auto node = frontNode->next; node != NULL; node = node->next)
		{
			auto child = passGraph[node->name];
			child->curInDegree--;
			if (child->curInDegree == 0)
				que.push(child);
		}
		que.pop();
	}

	RestoreState();
}

void PassGraph::Render(Pass* endPass) // reverse topological sorting and render
{
	std::string endName = endPass->name;
	queue<GraphNode*> que;
	stack<Pass*> renderPass;
	for (auto node : passGraph) // delete other leaf
	{
		if (node.second->curOutDegree == 0 && node.second->name != endName)
		{
			auto ptr = node.second->pre;
			while (NULL != ptr)
			{
				passGraph[ptr->name]->curOutDegree--;
				ptr = ptr->pre;
			}
		}
	}
	que.push(passGraph[endName]);

	while (!que.empty())
	{
		GraphNode* frontNode = que.front();
		assert(frontNode->curOutDegree == 0 && "que front in degree not zero!");
		renderPass.push(passes[frontNode->name]);
		for (auto node = frontNode->pre; node != NULL; node = node->pre)
		{
			auto child = passGraph[node->name];
			child->curOutDegree--;
			if (child->curOutDegree == 0)
				que.push(child);
		}
		que.pop();
	}

	RestoreState();

	//cout << "render order:" << endl;
	while (!renderPass.empty())
	{
		Pass* pass = renderPass.top();
		//cout << pass->as_pass->name << endl;
		pass->Render();
		renderPass.pop();
	}
}

void PassGraph::SetInputPass(Pass const* sourcePass, Pass const* targetPass)
{
	std::string srcName = sourcePass->name;
	std::string tarName = targetPass->name;
	
	auto ptr = passGraph[srcName];
	auto srcPtr = ptr;
	while (NULL != ptr->next)
	{
		if (ptr->name == targetPass->name) // avoid self edges and duplicate edges
			return;
		ptr = ptr->next;
	}
	if (ptr->name == targetPass->name) // avoid self edges and duplicate edges
		return;

	//next added
	GraphNode* node = passGraph[tarName];
	ptr->next = new GraphNode(tarName);

	//pre added
	ptr = node;
	while (NULL != ptr->pre) ptr = ptr->pre;
	ptr->pre = new GraphNode(srcName);

	// in/out degree
	srcPtr->outDegree++;
	srcPtr->curOutDegree = srcPtr->outDegree;
	node->inDegree++;
	node->curInDegree = node->inDegree;
}

void PassGraph::SetInputTexture(Texture*& sourceTexture, Texture*& tergetTexture)
{
	tergetTexture = sourceTexture;
}

void PassGraph::SetInput(Pass const* sourcePass, Pass const* targetPass, Texture*& sourceTexture, Texture*& targetTexture)
{
	SetInputPass(sourcePass, targetPass);
	SetInputTexture(sourceTexture, targetTexture);
}

void PassGraph::DeleteInput(Pass const* sourcePass, Pass const* targetPass)
{
	
	std::string srcName = sourcePass->name;
	std::string tarName = targetPass->name;
	if (passGraph.find(srcName) == passGraph.end())
		return;
	auto ptr = passGraph[srcName], srcPtr = ptr;
	auto s = ptr;
	while (NULL != ptr->next)
	{
		ptr = ptr->next;
		if (ptr->name == tarName) // delete
		{
			srcPtr->curOutDegree--;
			srcPtr->outDegree--;
			s->next = ptr->next;
			delete ptr;
		}
		else
			s = s->next;
		ptr = s;
	}

	ptr = passGraph[tarName];
	auto tarPtr = ptr;
	s = ptr;
	while (NULL != ptr->pre)
	{
		ptr = ptr->pre;
		if (ptr->name == srcName) // delete
		{
			tarPtr->curInDegree--;
			tarPtr->inDegree--;
			s->pre = ptr->pre;
			delete ptr;
		}
		else
			s = s->pre;
		ptr = s;
	}
}

void PassGraph::AttachPass(Pass* pass)
{
	std::string name = pass->name;
	this->passGraph.insert({name, new GraphNode(name)});
	this->passes.insert({name, pass});
}

Pass* PassGraph::MergePass(Pass* originPass, Pass* targetPass)
{
	hashMergeBuffer.push_back(0);
	originPass->bufferID = (int)hashMergeBuffer.size() - 1;
	targetPass->bufferID = originPass->bufferID;
	originPass->renderTarget = targetPass->renderTarget;
	targetPass->mClearState.clearFlag = false;
	SetInputPass(originPass, targetPass);
	return targetPass;
}

void PassGraph::PushEffect(PipelineEffect* effect)
{
	this->effects.insert({ effect->name, effect });
}

void PassGraph::AttachEffect(PipelineEffect* effect)
{
	PushEffect(effect);
	vector<Pass*> passes;
	vector<pair<Pass*, Pass*>> inputPass;
	vector<pair<Texture*&, Texture*&>> inputTexture;
	effect->GetInputPasses(passes, inputPass, inputTexture);
	for(auto pass : passes)
		AttachPass(pass);
	auto passSize = (int)inputPass.size();
	auto texSize = (int)inputTexture.size();
	for (int i = 0; i < passSize; ++i)
		SetInputPass(inputPass[i].first, inputPass[i].second);
	for (int i = 0; i < texSize; ++i)
		SetInputTexture(inputTexture[i].first, inputTexture[i].second);
}

void PassGraph::AttachAllEffects()
{
	for (auto e : effects)
		AttachEffect(e.second);
}

void PassGraph::DetachEffect(PipelineEffect* effect)
{
	this->effects.erase(effect->name);

	vector<Pass*> passes;
	vector<pair<Pass*, Pass*>> inputPass;
	vector<pair<Texture*&, Texture*&>> inputTexture;
	effect->GetInputPasses(passes, inputPass, inputTexture);
	auto passSize = (int)inputPass.size();
	for (int i = 0; i < passSize; ++i)
	{
		DeleteInput(inputPass[i].first, inputPass[i].second);
	}
}

void PassGraph::PrintGraph()
{
	for (auto i : this->passGraph)
	{
		cout << i.first << endl;
		std::cout <<"in degree "<< i.second->inDegree << " ";
		std::cout << i.second->name << "--->";
		auto j = i.second->next;
		while (NULL != j)
		{
			std::cout << j->name << "--->";
			j = j->next;
		}
		std::cout << "NULL" << endl;
	}

}

void PassGraph::RestoreState()
{
	fill(hashMergeBuffer.begin(), hashMergeBuffer.end(), 0);
	for (auto node : passGraph)
	{
		node.second->curInDegree = node.second->inDegree;
		node.second->curOutDegree = node.second->outDegree;
	}
}

void PassGraph::CollectObject(const SceneManager* scene)
{
	const map<string, vector<string>> & effectList = scene->getEffectList();
	
	for (auto& object : effectList) // for each object
	{
		auto& objectList = object.second;
		for (auto& effect : objectList) // for each effect in object
		{
			string name = effect;
			scene->pushNodeToQueue(object.first, this->effects[name]->queue);
		}
	}
}
