#pragma once
#include "ResourceManager.h"
class Sampler{
public:
	string name;
	enum TextureFilter
	{
		TextureFilter_Nearest,
		TextureFilter_Linear,
		TextureFilter_Nearest_Mipmap_Nearest,
		TextureFilter_Linear_Mipmap_Nearest,
		TextureFilter_Nearest_Mipmap_Linear,
		TextureFilter_Linear_Mipmap_Linear
	};

	enum SamplerWrap
	{
		TextureWrap_Clamp,
		TextureWrap_Repeat,
		TextureWrap_Mirror
	};
	enum SamplerCompareMode {
		NONE,
		COMPARE_REF_TO_TEXTURE
	};
	enum SamplerCompareFunc {
		NEVER, 
		ALWAYS, 
		LESS, 
		LEQUAL, 
		EQUAL, 
		NOT_EQUAL, 
		GEQUAL, 
		GREATER
	};
	TextureFilter
		min_filter,
		mag_filter;

	SamplerCompareMode cmpmode=NONE;
	SamplerCompareFunc cmpfunc;

	SamplerWrap
		wrap_s, 
		wrap_t,
		wrap_r;

	Sampler(){
		min_filter = TextureFilter_Linear;
		mag_filter = TextureFilter_Linear;
		wrap_s = TextureWrap_Repeat;
		wrap_t = TextureWrap_Repeat;
		wrap_r = TextureWrap_Repeat;
	}
	virtual void createInternalRes() = 0;
	virtual void bind(int texture_unit) = 0;

	void setRepeatLinear(){
		min_filter = TextureFilter_Linear;
		mag_filter = TextureFilter_Linear;
		wrap_s = TextureWrap_Repeat;
		wrap_t = TextureWrap_Repeat;
		wrap_r = TextureWrap_Repeat;
	}
	void setRepeatPoint(){
		min_filter = TextureFilter_Nearest;
		mag_filter = TextureFilter_Nearest;
		wrap_s = TextureWrap_Repeat;
		wrap_t = TextureWrap_Repeat;
		wrap_r = TextureWrap_Repeat;
	}
	void setClampLinear(){
		min_filter = TextureFilter_Linear;
		mag_filter = TextureFilter_Linear;
		wrap_s = TextureWrap_Clamp;
		wrap_t = TextureWrap_Clamp;
		wrap_r = TextureWrap_Clamp;

	}
	void  setClampPoint(){

		min_filter = TextureFilter_Nearest;
		mag_filter = TextureFilter_Nearest;
		wrap_s = TextureWrap_Clamp;
		wrap_t = TextureWrap_Clamp;
		wrap_r = TextureWrap_Clamp;

	}
	void setRepeatLinearMipLinear(){
		min_filter = TextureFilter_Linear_Mipmap_Linear;
		mag_filter = TextureFilter_Linear;
		wrap_s = TextureWrap_Repeat;
		wrap_t = TextureWrap_Repeat;
		wrap_r = TextureWrap_Repeat;
	}

};

class  SamplerFactory
{
public:
	virtual Sampler * create() = 0;

};

class SamplerManager :public ResourceManager<Sampler*>{
public:
	static SamplerManager& getInstance()
	{
		static SamplerManager    instance;
		return instance;
	}
	Sampler* createSampler(string name){
		Sampler* s=GlobalResourceManager::getInstance().m_SamplerFactory->create();
		s->name = name;
		add(s);
		return s;
	}
private:
	SamplerManager(){
		Sampler* s=createSampler("RepeatLinear");
		s->setRepeatLinear();

		s = createSampler("RepeatPoint");
		s->setRepeatPoint();

		s = createSampler("ClampLinear");
		s->setClampLinear();

		s = createSampler("ClampPoint");
		s->setClampPoint();

		s = createSampler("RepeatLinearMipLinear");
		s->setRepeatLinearMipLinear();

		s = createSampler("ClampLinearMipLinear");
		s->setRepeatLinearMipLinear();

		s = createSampler("ShadowMap");
		s->setClampLinear();
		s->min_filter = Sampler::TextureFilter_Linear;
		s->cmpmode = Sampler::COMPARE_REF_TO_TEXTURE;
		s->cmpfunc = Sampler::LEQUAL;
	}
	SamplerManager(SamplerManager const&);              // Don't Implement.
	void operator=(SamplerManager const&); // Don't implement
};
