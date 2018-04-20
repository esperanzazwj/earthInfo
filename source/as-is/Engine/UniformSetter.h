#pragma once
#include "PreDefine.h"
#include "GpuProgram.h"
#include "ResourceManager.h"
#include "RenderSystem.h"
#include "Texture.h"
#include "pass.h"

class UniformSetter {
public:
	void SetProgramTexture(const string& name, GpuProgram* program, Texture* tex, int index) {
		auto rhi = GlobalResourceManager::getInstance().m_RenderSystem;
		rhi->BindTexture(tex, index);
		rhi->SetUniform(name, program, "int", 1, &index);
	}
	void SetTexureSampler(const string& name, Pass* pass, Texture* tex, int index) {
		auto rhi = GlobalResourceManager::getInstance().m_RenderSystem;
		Sampler* s = pass->getSampler(name);
		if (s == NULL)
			s = tex->texture_sampler;
		rhi->BindSampler(s, index);
	}
};

