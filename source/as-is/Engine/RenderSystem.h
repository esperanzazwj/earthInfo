#pragma once
#include <string>
#include "mathlib.h"
#include "RenderSystemEnum.h"
#include "SceneManager.h"
#include "Geometry.h"
#include "RenderState.h"
#include "PreDefine.h"

namespace HW
{
	class RenderSystem
	{
	public:
		RenderSystem(){}

		virtual ~RenderSystem() {}

		/** Set the width and height of client window
		*/
		virtual void SetWandH(int w, int h) = 0;
		virtual void GetWandH(int& w, int& h) = 0;

		/** initialize the render system. create device and device context.
		*/
		virtual bool  Initialize() = 0;

		virtual void InitFactory() = 0;

		virtual void  BindInputLayout(InputLayout *layout) = 0;

		virtual void  SetPrimType(PrimType geoType = PRIM_TYPE_TRIANGLES) = 0;

		virtual void  BindGpuProgram(GpuProgram *program) = 0;

		virtual void EnableWireFrame(bool op = false)=0;

		virtual void SetCullMode(CullMode md = CULL_MODE_BACK)=0;

		virtual void SetFaceFront(bool clockwise = false)=0;

		virtual void EnableMultiSample(bool ms = false)=0;	// if ms == true then multi-sample enabled,vice versa

		virtual void EnableScissorTest(bool teston = false) = 0;

		virtual void SetScissorRect(int tplx,int tply,int btrx,int btry)=0;

		virtual void RasterParamDefault()=0;

		virtual void SetViewport(float topLeftX,float topLeftY,float width,float height,float mindepth = 0.0f,float maxdepth = 1.0f)=0;

		virtual void EnableBlendAlphaCoverage(bool bac = false)=0;

		virtual void EnableIndependentBlend(bool ib = false)=0; 	// enable independent blend between multi-render target
		/** set blend parameters
		*/
		virtual void SetBlendParam(unsigned int target_index,BlendOperand srcBlend,
			BlendOperand destBlend,BlendOperation blendOp,BlendOperand srcAlpha,
			BlendOperand destAlpha,BlendOperation alphaOp,unsigned char rtMask)=0;
		/** turn on / off blend for a specific render target.
		*/
		virtual void EnableBlend(unsigned int rendertarget_index,bool blend = false) =0;
		/** set blend parameters to default values.this function is used
			to recovery the render state.
		*/
		virtual void BlendParamDefault() = 0;
		/** set blend factor.
		*/
		virtual void SetBlendFactor(float const bf[4],unsigned int sampleMask)=0;
		/** set depth test parameter
		*/
		virtual void SetDepthParameters(ComparisonFunc func,bool mask)=0;
		/** enable/disable depth test
		*/
		virtual void EnableDepthTest(bool teston = true) = 0;
		/** set stencil parameters when stencil test is applied.
		*/
		virtual void SetStencilParameters(unsigned char readmask,unsigned char writemask,StencilOperation frontFailOp,
			StencilOperation frontdepthfailOp,StencilOperation frontPassOp,ComparisonFunc front,
			StencilOperation backFailOp,StencilOperation backdepthfailOp,StencilOperation backPassOp,
			ComparisonFunc back,unsigned refv)=0;

		virtual void EnableStencil(bool teston = false) = 0;

		virtual void DepthClearParam(float depth = 1.0f,unsigned char stencil = 0)=0;

		virtual bool PreRender()=0;

		virtual void ClearBuffer(ClearState& c)=0;

		virtual void SetRenderTarget(RenderTarget *rt)=0;

		virtual void SetClearColor(float r, float g, float b, float a) = 0;

		virtual void setClearState(const ClearState &clearstate) = 0;
		virtual void setBlendState(const BlendState &blendstate) = 0;
		virtual void setRasterState(const RasterState & rasterstate) =0 ;
		virtual void setDepthStencilState(const DepthStencilState & depthstencilstate)=0;
		virtual void setPolygonOffset(bool enable, float offset_factor = 0.0, float offset_unit = 0.0) = 0;



		virtual void CreateGeometryGpuData(as_Geometry* /*rawdata*/, VertexDataDesc* /*inputlayout*/, Geometry*& /*geo*/){}
		virtual void DrawGeometry(Geometry* /*geo*/){}
		virtual void DrawGeometryInstance(Geometry* /*geo*/,int /*num*/) {}

		// about send uniforms and textures to program
		virtual void BindTexture(Texture* /*texture*/, int /*index*/){}
		virtual void BindSampler(Sampler* /*sampler*/, int /*index*/) {}
		// call after set program
		virtual void SetUniform(const string& /*name*/,GpuProgram* /*program*/,string /*datatype*/, unsigned int /*count*/, void* /*val*/) {}


		virtual void term_display() = 0;

	};
}

