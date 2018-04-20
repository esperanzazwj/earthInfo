#pragma once
#include<string>
#include "mathlib.h"
#include "NameGenerator.h"
#include "PreDefine.h"
#include "RenderItem.h"
namespace HW
{
	
	class Movable
	{
	public:
		enum MovableType
		{
			MT_UNKNOWN = 0,
			MT_ENTITY = 1,
			MT_LIGHT = 2,
			MT_CAMERA = 3
		};

		Movable();

		Movable(const string& name,MovableType type,SceneManager * m_creator =NULL);

		virtual ~Movable(){

		}

		void setParent(SceneNode* parent) { m_parent = parent; }
		SceneNode* getParent()const { return m_parent; }

		void setName(const string& name) { m_Name = name; }

		const string& getName()const {return m_Name; }

		void setMovableType(MovableType type) { m_MovableType = type; }

		MovableType getMovableType() const { return m_MovableType; }

		bool visible() const { return m_Visible; }

		void setVisible(bool isvisible = true) { m_Visible = isvisible; }
		/** tell this object that it has been moved.
		*/
		void notifyMoved() { moveActionImp(); }

		inline const BoundingBox& getBoundingBox() 
		{
			getBoundingBoxImp();
			return m_boundingBox;
		}

		inline const Sphere& getBoundingSphere()
		{
			getBoundingSphereImp();
			return m_sphere;
		}

		inline bool isMoved()
		{
			return m_moved;
		}
		inline SceneManager * getCreater()
		{
			return m_SceneMgr;
		}
		virtual void updateRenderqueue(Camera * ,RenderQueue & /*renderqueue*/)
		{

		}
	protected:
		/** action to be taken when this object is moved.
		*/
		virtual void moveActionImp()=0;
		virtual void getBoundingBoxImp() = 0;
		virtual void getBoundingSphereImp() = 0;
	protected:

		BoundingBox m_boundingBox;

		Sphere m_sphere;

		string m_Name;

		MovableType m_MovableType;

		bool m_Visible;
		// pointer to the parent node.
		SceneNode* m_parent;
		/** a name generator to generate default name for a newly created object.
		*/
		static NameGenerator* m_NameGenerator;
		SceneManager* m_SceneMgr;

		bool m_moved;
	};
}
