#pragma once

#include "RenderTarget.h"
#include "SceneManager.h"
#include "engine_struct.h"
#include "ResourceManager.h"
#include "pass.h"
#include "RenderSystem.h"
#include "TextureManager.h"


class RenderUtil {
public:
	static RenderQueue GetScreenQuad() {
		auto quad = GlobalResourceManager::getInstance().as_meshManager.get("ScreenQuad.obj");
		RenderQueueItem item;
		item.asMesh = quad;
		RenderQueue queue2;
		queue2.push_back(item);
		return queue2;
	}

	static void GetScreenQuad(RenderQueue& renderqueue) {
		auto quad = GlobalResourceManager::getInstance().as_meshManager.get("ScreenQuad.obj");
		RenderQueueItem item;
		item.asMesh = quad;
		renderqueue.push_back(item);
	}
};