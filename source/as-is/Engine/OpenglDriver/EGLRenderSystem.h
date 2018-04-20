#pragma once
#include "../RenderSystem.h"
#include "OPENGL_Include.h"
#include "../mathlib.h"
#include <stack>

namespace HW
{
	class EGLRenderSystem : public RenderSystem
	{
	public:
		EGLRenderSystem();
		virtual ~EGLRenderSystem() {
			term_display();
		}
		virtual bool  Initialize();

		void InitFactory();
		/** bind an input layout to the render system.
			@remarks
				we should tell the system the variable layout of the input vertex data
			before vertex data are streamed into the pipeline.
			@param layout  a struct describes the input variables' layout in memory.
		*/
		virtual void  BindInputLayout(InputLayout * /*layout*/) {}
		/** set primitive type for the to be rendered geometry.
		*/
		virtual void  SetPrimType(PrimType /*geoType*/ = PRIM_TYPE_TRIANGLES) {}
		/** bind a gpu program to the system
		*/
		virtual void  BindGpuProgram(GpuProgram *program);
		/** a switch to decide if wire frame is applied.
			@param op  a bool value if it is true then wire frame is enabled,vice versa.
		*/
		virtual void EnableWireFrame(bool /*op*/ = false) {} 	// if op == true then wire frame enabled,vice versa
		/** set cull model. #see CullModel (file: LoongRenderSystemEnum.h)
		*/
		virtual void SetCullMode(CullMode md = CULL_MODE_BACK);
		/** set which sides of a face will be thought as front face.
			@param clockwise  the direction for a face
		*/
		virtual void SetFaceFront(bool clockwise = false);
		/** decide if multi-sample technique should be applied
		*/
		virtual void EnableMultiSample(bool /*ms*/ = false) {}	// if ms == true then multi-sample enabled,vice versa
		/** a switch function to turn on/off the scissor test.
			@param teston  if it is true,then scissor test may be applied when rendering.
				the default value is false.
		*/
		virtual void EnableScissorTest(bool teston = false);
		/** parameters for scissor test.
		*/
		virtual void SetScissorRect(int tplx, int tply, int btrx, int btry);
		/** set raster state to default.
		*/
		virtual void RasterParamDefault() {}
		/** set view port parameters
		*/
		virtual void SetViewport(float topLeftX, float topLeftY, float width, float height, float mindepth = 0.0f, float maxdepth = 1.0f);
		/**
		*/
		virtual void EnableBlendAlphaCoverage(bool /*bac*/ = false) {}
		/** enable/disable independent blend when multi-render target is applied.
			warning : in opengl es 2.0 there is no mrt technique
		*/
		virtual void EnableIndependentBlend(bool /*ib*/ = false) {}	// enable independent blend between multi-render target
		/** set blend parameters
		*/
		virtual void SetBlendParam(unsigned int target_index, BlendOperand srcBlend,
			BlendOperand destBlend, BlendOperation blendOp, BlendOperand srcAlpha,
			BlendOperand destAlpha, BlendOperation alphaOp, unsigned char rtMask);
		/** turn on / off blend for a specific render target.
			in opengL es there no multiple render target
		*/
		virtual void EnableBlend(unsigned int rendertarget_index, bool blend = false);
		/** set blend parameters to default values.this function is used
			to recovery the render state.
		*/
		virtual void BlendParamDefault() {}
		/** set blend factor.
		*/
		virtual void SetBlendFactor(float const bf[4], unsigned int sampleMask);
		/** set depth test parameter
		*/
		virtual void SetDepthParameters(ComparisonFunc func, bool mask);
		/** enable/disable depth test
		*/
		virtual void EnableDepthTest(bool teston = true)
		{
			if (teston)
				glEnable(GL_DEPTH_TEST);
			else
				glDisable(GL_DEPTH_TEST);

		}
		/** set stencil parameters when stencil test is applied.
		*/
		virtual void SetStencilParameters(unsigned char readmask, unsigned char writemask, StencilOperation frontFailOp,
			StencilOperation frontdepthfailOp, StencilOperation frontPassOp, ComparisonFunc front,
			StencilOperation backFailOp, StencilOperation backdepthfailOp, StencilOperation backPassOp,
			ComparisonFunc back, unsigned refv);
		/** enable/disable stencil test
		*/
		virtual void EnableStencil(bool teston = false);
		/** set depth and stencil buffer parameters to default.
		*/
		virtual void DepthStencilDefault() {}
		/** set all parameters within this pipeline to default values.
		*/
		virtual void AllParamDefault() {}
		/** set back ground color.the default value is Vector4f::ZERO
		*/

		virtual void DepthClearParam(float depth = 1.0f, unsigned char stencil = 0)
		{
			glClearDepthf(depth);
			glClearStencil(stencil);
		}
		/** do some internal initialization before actual rendering.
		*/
		virtual bool PreRender() { return true; }
		/** swap back buffer to the front.and present the frame buffer to the monitor.
		*/

		virtual void ClearBuffer(ClearState& c);
		/** do some internal recovery after rendering.
		*/
		virtual bool PostRender() { return true; }
		/** return the name of this render system.
		*/
		virtual string GetName() const { return string("opengl es 3.0"); }

		virtual void SetRenderTarget(RenderTarget *rt);

		virtual void SetClearColor(float r, float g, float b, float a);
		virtual void setClearState(const ClearState &clearstate);
		virtual void setRasterState(const RasterState & rasterstate);
		virtual void setBlendState(const BlendState &blendstate);
		virtual void setDepthStencilState(const DepthStencilState & depthstencilstate);

		virtual void setPolygonOffset(bool enable = false, float offset_factor = 0.0, float offset_unit = 0.0);

		void SetWandH(int w, int h) { width = w; height = h; }
		void GetWandH(int &w, int &h) { w = width; h = height; }
		virtual void CreateGeometryGpuData(as_Geometry* rdata, VertexDataDesc* inputlayout, Geometry*& geo);
		virtual void DrawGeometry(Geometry* geo);
		virtual void DrawGeometryInstance(Geometry* geo, int num);

		// about send uniforms and textures to program
		virtual void BindTexture(Texture* texture, int index);
		virtual void BindSampler(Sampler* sampler, int index);
		// call after set program 
		virtual void SetUniform(const string& name, GpuProgram* program, string datatype, unsigned int count, void* val);

		void term_display();

	private:
		GLint width;
		GLint height;
	};
}
