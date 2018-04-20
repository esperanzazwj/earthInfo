#pragma once
#include "RenderSystem.h"
#include "PreDefine.h"

class RenderStrategy {
public:
	void RenderPass_old(Camera* camera, RenderQueue & renderqueue, as_Pass* pass, RenderTarget * rt);

	void RenderPass(Camera* camera, RenderQueue & renderqueue, Pass* pass, RenderTarget * rt);

	void RenderPassMini(Camera* camera, RenderQueue & renderqueue, as_Pass* pass, RenderTarget * rt);

	void SetPassState(Camera* camera, Pass* pass, RenderTarget * rt);


};