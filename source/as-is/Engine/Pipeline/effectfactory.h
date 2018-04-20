#pragma once
#include "../effect.h"
#include "GlobePipeline.h"
#include "PassGraph.h"
#include <map>
#include <string>
#include <stdexcept>

class EffectFactory
{
public :

	static EffectFactory& GetInstance()
	{
		static EffectFactory eFactory;
		return eFactory;
	}

	/*
	** 1. create effect and insert into effect map for later look up
	** 2. effect mask is encoded in the following format : effect mask ## light mask
	** 3. objects hold mask each to specify what effect they are influenced by
	** 4. effect hold mask to specify itself
	*/
	PipelineEffect* CreateEffect(string name, int lightSize)
	{
		if (effectMap.find(name) != effectMap.end())
			return effectMap[name];
		effectList.push_back(name);

		auto effect = [&] () -> PipelineEffect* {
			if (name == "globe")
				return new GlobeEffect;

			throw std::runtime_error{"no such effect: " + name};
		} ();

		unsigned int mask = 0x00000001u;
		effect->name = name;
		effect->mask = mask << (id++) << lightSize;
		effectMap.insert({name, effect});
		return effect;
	}


	std::unordered_map<string, PipelineEffect*>& AllEffects()
	{
		return effectMap;
	}

	PipelineEffect* GetEffect(const string& name)
	{
		if (effectMap.find(name) != effectMap.end())
			return effectMap[name];
		return NULL;
	}

	unsigned int GetEffectNum()
	{
		return id;
	}

	inline void PushEffects(PassGraph& pg)
	{
		for (auto e : effectMap)
		{
			pg.PushEffect(e.second);
		}
	}

	std::vector<string> effectList;
private :

	// --- private constructors & copy constructors ---
	EffectFactory() {};
	EffectFactory(const EffectFactory&) {};

	// --- unique effect id ---
	unsigned int id = 0;

	// --- effect map ---
	std::unordered_map<std::string, PipelineEffect*> effectMap;
};

