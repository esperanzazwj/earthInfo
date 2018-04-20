#pragma once
#include "../RenderTarget.h"
namespace HW
{
	class RenderSystem;
	class EGLRenderWindow : public RenderTarget
	{
	public:
		EGLRenderWindow() {}
		EGLRenderWindow(const string & name,RenderSystem * render);
		virtual ~EGLRenderWindow(){}


		virtual void createInternalRes(){}
		virtual void releaseInternalRes(){}
		//virtual void setWindowPos(unsigned int ix,unsigned int iy){}
		virtual void bindTarget(int index = 0);
	private:
		unsigned int m_width;
		unsigned int m_height;
	};
}
