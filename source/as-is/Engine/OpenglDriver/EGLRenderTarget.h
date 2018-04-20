#pragma once
#include "../RenderTarget.h"
#include "OPENGL_Include.h"
#include <string>

namespace HW
{
	class EGLRenderTarget final : public RenderTarget
	{
	public:
		void createInternalRes() override;
		void releaseInternalRes() override;
		void bindTarget(int index = 0) override;

		void attachTextureCubeMapNow(Texture* cubetexture, string attach_component, int face) override;
		void attachTexture(Texture*, string attach_component) override;

		void RebindAttachTextures_Internal() override;
		void BlitTexture(RenderTarget* out, string SrcAttach, string DstAttach) override;
		GLuint m_framebuffer_id;

	private:
		void createFrameBuffer();
		GLenum getAttach(std::string const& attach);
	};
}

class EGLRenderTargetFactory final : public RenderTargetFactory
{
public:
	RenderTarget * create() override {
		return new EGLRenderTarget();
	}
};
