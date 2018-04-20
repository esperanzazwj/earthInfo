#pragma once
#include "Pipeline/PassGraph.h"
#include <map>
using std::map;

class Pipeline{
public:
	virtual void Init() = 0;
	virtual void Render() {}
	PassGraph passGraph;
};

