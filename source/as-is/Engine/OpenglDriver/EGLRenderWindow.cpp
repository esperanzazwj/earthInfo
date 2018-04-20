#include "EGLRenderWindow.h"
#include "EGLRenderSystem.h"
#include "../ResourceManager.h"
#include "EGLUtil.h"

namespace HW
{
	EGLRenderWindow::EGLRenderWindow(const string & name,RenderSystem * /*render*/)
		:RenderTarget(name)
	{
		m_width = m_height = 0;
	}
	//unsigned int EGLRenderWindow::getWidth() const 
	//{
	//	if(m_width != 0)
	//		return m_width;
	//	 int  width ,height;
	//	auto system=GlobalResourceManager::getInstance().m_RenderSystem;
	//	//EGLRenderSystem * system = dynamic_cast<EGLRenderSystem *>(m_RenderSystem);
	//	system->GetWandH(width,height);
	//	return width;
	//}

	//unsigned int EGLRenderWindow::getHeight() const
	//{
	//	if(m_height)
	//		return m_height;
	//	int  width ,height;
	//	auto system = GlobalResourceManager::getInstance().m_RenderSystem;
	//	//EGLRenderSystem * system = dynamic_cast<EGLRenderSystem *>(m_RenderSystem);
	//	system->GetWandH(width,height);
	//	return height;
	//}

	void EGLRenderWindow::bindTarget(int /*index*/)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		GL_CHECK();
	}
}
