#pragma once
#include "Movable.h"
#include "OMesh.h"
#include "light.h"
//#include "AnimationState.h"
#include <list>
#include "PreDefine.h"

namespace HW
{
	class Camera;
	/** 
	*/
	class Entity : public Movable
	{
	public:
		// never use this constructor.
		Entity():m_CastShadow(true){}

		Entity(const string &name,SceneManager * creator = NULL);

		~Entity();

		void setMesh(MeshPtr mesh) ;

		//AnimationState * getAnimationState()
		//{
		//	if(!animationstate)
		//	{
		//		return NULL;
		//	}
		//	return animationstate;
		//}
		//

		MeshPtr getMesh() const { return m_Mesh; }

		//void  setMaterial(MaterialPtr material);

		const std::map<string,Light*>& getLightList() const { return m_LightList; }

		void recordLight(Light *light) { 
			if(m_LightList.find(light->getName()) == m_LightList.end())
				m_LightList.insert(std::make_pair(light->getName(),light));
		}
		void clearLightList() { m_LightList.clear(); }

		bool isCastShadow()
		{
			return m_CastShadow;
		}
		/**
		set this entity castShadow or Not
		*/
		void setCastShadow(bool cast)
		{
			m_CastShadow = cast;
		}

		/**
		effect mask
		*/
		void SetEffectMask(unsigned int mask)
		{
			this->effectMask = mask;
		}

		unsigned int * GetEffectMask()
		{
			return &(this->effectMask);
		}

		virtual void updateRenderqueue(Camera * came,RenderQueue & renderqueue);
		virtual void updateRenderqueue_as(Camera * came, RenderQueue & renderqueue);
		void setEffect(Effect *effect)
		{
			m_Mesh->setLocalEffect(effect);
		}

	protected:
		// set bounding box and bounding sphere invalid.
		virtual void moveActionImp()
		{
			m_boundingBoxCurrent = false;
			m_boundingSphereCurrent = false;
		}
		// transform bounding box from model space to global space.
		virtual void getBoundingBoxImp();
		// transform bounding sphere from model space to global space.
		virtual void getBoundingSphereImp();

		// hint bounding box
		bool m_boundingBoxCurrent;
		bool m_boundingSphereCurrent;

		MeshPtr  m_Mesh;

		//AnimationState * animationstate;
		/** a list of light which influence this entity.
		*/
		std::map<string,Light*> m_LightList;
		unsigned int effectMask;

		bool m_CastShadow;
	};
}
