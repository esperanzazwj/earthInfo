#pragma once
#include "PreDefine.h"
#include <vector>
#include <algorithm>
#include "engine_struct.h"

namespace HW
{
	struct RenderQueueItem
	{
		Entity* entity{};
		as_Mesh* asMesh{};
		unsigned int sortkey={};
		Geometry* geometry{};
	};

	typedef std::vector<RenderQueueItem> RenderQueue;
	inline bool namecompare(const RenderQueueItem & a, const RenderQueueItem & b)
	{
		if (a.asMesh == NULL || b.asMesh == NULL) return false;
		return a.asMesh->name < b.asMesh->name;  
	}
	inline void SortByMeshName(RenderQueue& queue) {
        std::sort(queue.begin(), queue.end(), namecompare);
	}

	//inline bool CompDistRenderQueueItem(const RenderQueueItem & a,const RenderQueueItem & b)
	//{
	//	
	//	return a.sortkey < b.sortkey;
	//	
	//}
}

