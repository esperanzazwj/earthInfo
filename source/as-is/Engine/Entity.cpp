#include "Entity.h"
#include "SceneNode.h"
//#include "GlobalVariablePool.h"
#include "GpuProgram.h"
#include "RenderSystem.h"
//#include "SysEntry.h"
#include "RenderTarget.h"
#include "SceneManager.h"
#include "Camera.h"
namespace HW
{
	Entity::Entity(const string &name, SceneManager * creator)
		: Movable(name, MT_ENTITY, creator), m_CastShadow(true)
	{
		m_boundingBoxCurrent = false;
		m_boundingSphereCurrent = false;
		//animationstate = NULL;
	}

	Entity::~Entity(){
		//if (animationstate)
		//	delete animationstate;
	}



	void Entity::getBoundingBoxImp()
	{
		if (!m_boundingBoxCurrent)
		{
			if (m_parent) //
			{
				// const BoundingBox & modelbound  = m_Mesh->getModelBoundingBox();
				m_boundingBox = m_Mesh->getModelBoundingBox();
				m_boundingBox.transformAffine(m_parent->getWorldMatrix());

			}

			m_boundingBoxCurrent = true;
		}
	}

	void Entity::getBoundingSphereImp()
	{
		if (!m_boundingSphereCurrent)
		{
			if (m_parent)
			{
				Point3 center = m_Mesh->getModelBoundingSphere().center;
				// transform, scaling is not supported.
				center = m_parent->getWorldMatrix() * center;

				Vector3 scaled = m_parent->getScale();
				float max_scale = max(scaled.x, scaled.y);
				max_scale = max(scaled.z, max_scale);

				m_sphere.radius = m_Mesh->getModelBoundingSphere().radius*max_scale;
				m_sphere.center = center;
			}

			m_boundingSphereCurrent = true;
		}
	}

	void Entity::setMesh(MeshPtr mesh)
	{
		m_Mesh = mesh;

	}


	void Entity::updateRenderqueue(Camera * /*cam*/, RenderQueue & /*render_queue*/)
	{
	}

	void Entity::updateRenderqueue_as(Camera * /*cam*/, RenderQueue & /*render_queue*/)
	{
		////TEST ： if the System is in Shadow Generation，test if this entity castShadow
		//if (m_Visible && ((SysEntry::getSingleton().mrender_stage == RS_Depth && m_CastShadow) || SysEntry::getSingleton().mrender_stage != RS_Depth))
		//{
		//	int cull = cam->intersects(getBoundingBox());
		//	if (cull == 0)
		//		return;
		//
		//	for (auto& x : m_Mesh->m_SubMeshList_as){
		//		RenderQueueItem item;
		//		item.entity = this;
		//		item.asMesh = x.second;
		//		render_queue.push_back(item);
		//
		//	}
		//}
	}

}

