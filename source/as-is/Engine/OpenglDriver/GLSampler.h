#pragma once
#include "../Sampler.h"
#include "OPENGL_Include.h"
#include "EGLUtil.h"
using namespace HW;
class GLSampler :public Sampler{
public:
	virtual void createInternalRes(){
		glGenSamplers(1, &sampler_id);
		setParam();
		m_valid = true;
	}
	void setParam(){
		glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, EGLUtil::Get(min_filter));
		glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, EGLUtil::Get(mag_filter));
		glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, EGLUtil::Get(wrap_s));
		glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, EGLUtil::Get(wrap_t));
		glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_R, EGLUtil::Get(wrap_r));
		
		//待修正
		if (cmpmode != NONE) {
			glSamplerParameteri(sampler_id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glSamplerParameteri(sampler_id, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		}
	}
	virtual void bind(int texture_unit){
		if (!m_valid)
			createInternalRes();
		glBindSampler(texture_unit,sampler_id);
	}

	void release(){
		glDeleteSamplers(1, &sampler_id);
	}
	GLuint sampler_id;
	bool m_valid=false;
};

class  GLSamplerFactory:public SamplerFactory
{
public:
	virtual GLSampler * create(){
		return new GLSampler();
	}

};
