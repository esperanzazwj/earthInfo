#include "EGLRenderSystem.h"
#include "../RenderTarget.h"
#include "EGLUtil.h"
#include "../RenderState.h"
#include "GLGeometry.h"
#include "EGLTexture.h"
#include "gl_program.h"
#include "../ResourceManager.h"
#include "EGLRenderTarget.h"
#include "GLSampler.h"
#include "../RenderStrategy.h"
#include "../PerObjectUniform.h"
#include "../UniformSetter.h"
#include "../../../wsi/window-system.hpp"

namespace HW
{

	EGLRenderSystem::EGLRenderSystem()
	{
		auto& ws = ss::Window_System::current();
		width = ws.width();
		height = ws.height();
	}

	bool EGLRenderSystem::Initialize()
	{
		//printGLString("Version", GL_VERSION);
		//printGLString("Vendor", GL_VENDOR);
		//printGLString("Renderer", GL_RENDERER);
		//printGLString("Extensions", GL_EXTENSIONS);

		InitFactory();
		return true;

	}

	void EGLRenderSystem::BindGpuProgram(GpuProgram *program)
	{
		if(program==NULL) return ;
		program->UseProgram();

	}

	void EGLRenderSystem::SetCullMode(CullMode md)
	{
		glEnable(GL_CULL_FACE);
		switch(md)
		{
		case CULL_MODE_BACK:
			glCullFace(GL_BACK);
			break;
		case CULL_MODE_FRONT:
			glCullFace(GL_FRONT);
			break;
		case CULL_MODE_NONE:
			glDisable(GL_CULL_FACE);
			break;
		case CULL_MODE_FRONT_AND_BACK:
			glCullFace(GL_FRONT_AND_BACK);
			break;
		default:break;
		}
	}

	void EGLRenderSystem::SetFaceFront(bool clockwise )
	{
		if(clockwise)
			glFrontFace(GL_CW);
		else
			glFrontFace(GL_CCW);
	}

	void EGLRenderSystem::EnableScissorTest(bool teston)
	{
		if(teston)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);
	}

	void EGLRenderSystem::SetScissorRect(int tplx,int tply,int btrx,int btry)
	{
		glScissor(tplx,tply,btrx -tplx, btry - tply);
	}
	
	void EGLRenderSystem::EnableBlend (unsigned int /*rendertarget_index*/,bool blend )
	{
		if (blend)
		{
			glEnable(GL_BLEND);
		}
		else
			glDisable(GL_BLEND);
	}

	void EGLRenderSystem::SetBlendParam(unsigned int /*target_index*/,BlendOperand srcBlend, BlendOperand destBlend,BlendOperation blendOp,BlendOperand srcAlpha, BlendOperand destAlpha,BlendOperation alphaOp,unsigned char /*rtMask*/)
	{
		glBlendFuncSeparate(EGLUtil::Get(srcBlend),EGLUtil::Get(destBlend),EGLUtil::Get(srcAlpha),EGLUtil::Get(destAlpha));
		glBlendEquationSeparate(EGLUtil::Get(blendOp),EGLUtil::Get(alphaOp));
	}	

	void EGLRenderSystem::SetDepthParameters(ComparisonFunc func,bool depthmask)
	{
		GLboolean mask = depthmask;
		
		glDepthFunc(EGLUtil::Get(func));
		glDepthMask(mask);
	}

	void EGLRenderSystem::SetBlendFactor(float const bf[4],unsigned int /*sampleMask*/)
	{
		glBlendColor(bf[0],bf[1],bf[2],bf[3]);
	}
	void EGLRenderSystem::SetViewport(float topLeftX,float topLeftY,float _width,float _height,float /*mindepth*/ /* = 0.0f */,float /*maxdepth*/ /* = 1.0f */)
	{
		glViewport((int)topLeftX, (int)topLeftY, (GLsizei)_width, (GLsizei)_height);
		GL_CHECK();
	}

	void EGLRenderSystem::SetStencilParameters(unsigned char /*readmask*/,unsigned char writemask,StencilOperation frontFailOp,
		StencilOperation frontdepthfailOp,StencilOperation frontPassOp,ComparisonFunc front, StencilOperation backFailOp,
		StencilOperation backdepthfailOp,StencilOperation backPassOp, ComparisonFunc back,unsigned refv)
	{
		//in the opengl es 2.0 ,only one mask exits so i decide use writemask 
		glStencilFuncSeparate(GL_FRONT,EGLUtil::Get(front),refv,writemask);
		glStencilFuncSeparate(GL_BACK,EGLUtil::Get(back),refv,writemask);
		glStencilOpSeparate(GL_FRONT,EGLUtil::Get(frontFailOp),EGLUtil::Get(frontdepthfailOp),EGLUtil::Get(frontPassOp));
		glStencilOpSeparate(GL_BACK,EGLUtil::Get(backFailOp),EGLUtil::Get(backdepthfailOp),EGLUtil::Get(backPassOp));

	}
	
	void EGLRenderSystem::EnableStencil(bool teston)
	{
		if(teston)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);
	}


	void EGLRenderSystem::ClearBuffer(ClearState& c)
	{
        if (c.mclearcolorbuffer) {
            float clear_value[4];
            glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_value);
            glClearBufferfv(GL_COLOR, 0, clear_value);
            GL_CHECK();
        }

		if (c.mcleardepthbuffer) {
            glClearBufferfv(GL_DEPTH, 0, &c.mClearDepth);
            GL_CHECK();
        }

		if (c.mclearstencilbuffer) {
            glClear(GL_STENCIL_BUFFER_BIT);
            GL_CHECK();
        }
	}

	void EGLRenderSystem::SetRenderTarget(RenderTarget * rt)
	{
		rt->bindTarget();
	}


	 void EGLRenderSystem::SetClearColor(float r,float g,float b,float a)
	 {
		
		 glClearColor(r,g,b,a);
		
		 GL_CHECK();
	 }

	 void EGLRenderSystem::setRasterState(const RasterState & rasterstate)
	 {
		 static RasterState m_last_raster_state ;
		if(rasterstate == m_last_raster_state)
			 return;
		 // possible way to optimized
		 // store a previous raster state so we can only change the states that changed 

		 this->SetCullMode(rasterstate.mCullMode);
		 this->SetFaceFront(rasterstate.mFrontCounterClockwise);
		 this->EnableScissorTest(rasterstate.mScissorEnable);
		 glColorMask(rasterstate.mColorWriteMask[0],rasterstate.mColorWriteMask[1],rasterstate.mColorWriteMask[2],rasterstate.mColorWriteMask[3]);
		m_last_raster_state = rasterstate;
	 }

	 void EGLRenderSystem::setBlendState(const BlendState &blendstate)
	 {
		 static BlendState m_last_blend_state;
		 // possible way to optimized
		 // store a previous blend state so we can only change the states that changed 
		if(blendstate == m_last_blend_state)
			 return;
		 this->EnableBlend(0,blendstate.mBlendEnable);
		 this->SetBlendFactor(blendstate.mFactor,blendstate.mSampleMask);
		 this->SetBlendParam(0,blendstate.mSrcBlend,blendstate.mDestBlend,blendstate.mBlendFunc,
			 blendstate.mSrcBlendAlpha,blendstate.mDestBlendAlpla,blendstate.mBlendFuncAlpha,blendstate.mWriteMask);
		 m_last_blend_state = blendstate;
	 }

	 void EGLRenderSystem::setDepthStencilState(const DepthStencilState & depthstencilstate)
	 {
		 static DepthStencilState m_last_depth_state;
		 if (m_last_depth_state == depthstencilstate)
		 {
			 return;
		 }
		 // depth
		 this->EnableDepthTest(depthstencilstate.mDepthEnable);
		 this->SetDepthParameters(depthstencilstate.mDepthFunc,depthstencilstate.mDepthWriteMask);
		 //stencil
		 this->EnableStencil(depthstencilstate.mStencilEnable);
		 this->SetStencilParameters(depthstencilstate.mStencilReadMask,
			 depthstencilstate.mStencilWriteMask,depthstencilstate.mFrontStencilFailFunc,
			 depthstencilstate.mFrontStencilDepthFailFunc,depthstencilstate.mFrontStencilPassFunc,
			 depthstencilstate.mFrontStencilFunc,depthstencilstate.mBackStencilFailFunc,
			 depthstencilstate.mBackStencilDepthFailFunc,depthstencilstate.mBackStencilPassFunc,
			 depthstencilstate.mBackStencilFunc,depthstencilstate.mStencilRef);
		 m_last_depth_state = depthstencilstate;

	 }
	 void EGLRenderSystem::setClearState(const ClearState & /*clearstate*/)
	 {
		 
	 }
	 void EGLRenderSystem::setPolygonOffset(bool enable, float offset_factor, float offset_unit)
	 {
		 if (enable)
		 {
			 glEnable(GL_POLYGON_OFFSET_FILL);
			 glPolygonOffset(offset_factor, offset_unit);
		 }
		 else
		 {
			 glDisable(GL_POLYGON_OFFSET_FILL);
		 }
	 }
	 void EGLRenderSystem::CreateGeometryGpuData(as_Geometry* rdata, VertexDataDesc* inputlayout, Geometry*& geo)
	 {
		 assert(geo == NULL);
		 geo=new GLGeometry;
		 geo->Create(rdata, inputlayout);
	 }
	 void EGLRenderSystem::DrawGeometry(Geometry* geo)
	 {
		 GLGeometry* g = static_cast<GLGeometry*>(geo);
		 g->Draw();
	 }

	 void EGLRenderSystem::DrawGeometryInstance(Geometry* geo, int num)
	 {
		 GLGeometry* g = static_cast<GLGeometry*>(geo);
		 g->DrawInstance(num);
	 }

	 void EGLRenderSystem::InitFactory()
	 {
		 auto& grm = GlobalResourceManager::getInstance();
		 grm.m_platform_info = "GL";
		 grm.m_RenderSystem = this;
		 grm.m_RenderStrategy = new RenderStrategy;
		 grm.m_TextureFactory = new EGLTextureFactory;
		 grm.m_GpuProgramFactory = new GLGpuProgramFactory;
		 grm.m_GeometryFactory = new GLGeometryFactory;
		 grm.m_RenderTargetFactory = new EGLRenderTargetFactory;
		 grm.m_SamplerFactory = new GLSamplerFactory;
		 grm.m_PerObjectUniformGetter = new PerObjectUniformGetter;
		 grm.m_UniformSetter = new UniformSetter;
	 }

	 void EGLRenderSystem::term_display()
	 {

	 }

	 void EGLRenderSystem::BindSampler(Sampler* sampler, int index)
	 {
		 if (sampler == NULL) return;
		 sampler->bind(index);
	 }

	 void EGLRenderSystem::BindTexture(Texture* texture, int index)
	 {
		 GL_CHECK();
		 glActiveTexture(GL_TEXTURE0 + index);
		 if (texture == NULL)
		 {
			 glBindTexture(GL_TEXTURE_2D, 0);
		 }
		 else
			 texture->useTexture();
	 }

	 void EGLRenderSystem::SetUniform(const string& name, GpuProgram* program, string datatype, unsigned int count, void* val)
	 {
		 GLGpuProgram* p = static_cast<GLGpuProgram*>(program);
		 int location = p->getUniformLocation(name.c_str());
		 if (location <0) return;

		 if (datatype == "mat4")
		 {
             glUniformMatrix4fv(location, count, false, static_cast<Matrix4*>(val)[0].ptr());
		 }
		 if (datatype == "mat3")
		 {
             glUniformMatrix3fv(location, count, false, static_cast<Matrix3*>(val)[0].ptr());
		 }

		 if (datatype == "vec4")
		 {
			 glUniform4fv(location, count, (GLfloat *)val);
			 return;
		 }
		 if (datatype == "ivec4")
		 {
			 glUniform4iv(location, count, (GLint *)val);
			 return;
		 }
		 if (datatype == "vec3")
		 {
			 glUniform3fv(location, count, (GLfloat *)val);
			 return;
		 }
		 if (datatype == "ivec3")
		 {
			 glUniform3iv(location, count, (GLint *)val);
			 return;
		 }
		 if (datatype == "vec2")
		 {
			 glUniform2fv(location, count, (GLfloat *)val);
			 return;
		 }
		 if (datatype == "ivec2")
		 {
			 glUniform2iv(location, count, (GLint *)val);
			 return;
		 }
		 if (datatype == "float")
		 {
			 glUniform1fv(location, count, (GLfloat *)val);
			 return;
		 }
		 if (datatype == "int")
		 {
			 glUniform1iv(location, count, (GLint *)val);
			 return;
		 }
		 if (datatype == "uint")
		 {
			 glUniform1uiv(location, count, (GLuint *)val);
			 return;
		 }
	 }

}

