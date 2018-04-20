#pragma once
#include "mathlib.h"
#include "Movable.h"
#include "ElemIterator.h"
#include "NameGenerator.h"
#include "Entity.h"
#include <cstring>
#include <list>
#include <vector>
#include <map>

#ifdef ANDROID
#include <android/log.h>
#endif

namespace HW
{
	// transformation is performed.translation and rotation is supported.
	class SceneManager;
	class SceneNode
	{
	public:
		typedef mapIterator<string,SceneNode*> NodeIterator;
		typedef mapIterator<string,Movable*> MovableIterator;
	public:
		// not used directly. for fast initialization.
		SceneNode();
		SceneNode(const string& name ,SceneManager * creater);

		virtual ~SceneNode()
		{
			//std::cout<<"SceneNode "<<m_Name<<" Destruct"<<std::endl;
		}

		void setName(const string& name) { m_Name = name; }
		const string& getName() const { return m_Name; }
		void setParent(SceneNode* parent) { m_parent = parent; }
		SceneNode* getParent() const { return m_parent; }
		// child node management
		virtual void attachNode(SceneNode* node)
		{
			assert(node);
			m_childNodes.insert(std::make_pair(node->getName(),node));
			node->setParent(this);
		}
		// detach a node by passing in a pointer
		inline SceneNode* detachNode(SceneNode* node)
		{
			assert(node);
			
			SceneNode* tmp = NULL;
			std::map<string,SceneNode*>::iterator it = m_childNodes.find(node->getName());
			if( it != m_childNodes.end())
			{
				tmp = it->second;
				m_childNodes.erase(it);
				node->setParent(NULL);
			}
			return tmp;
		}
		// detach a node by name
		inline SceneNode* detachNode(const string& name)
		{
			SceneNode* tmp = NULL;
			std::map<string,SceneNode*>::iterator it = m_childNodes.find(name);
			if( it != m_childNodes.end())
			{
				tmp = it->second; 
				m_childNodes.erase(it);
				tmp->setParent(NULL);
			}
			return tmp;
		}
		// get a child node by name
		inline SceneNode* getChildNode(const string& name) const
		{
			std::map<string,SceneNode*>::const_iterator it = m_childNodes.find(name);
			if (it != m_childNodes.end())
			{
				return it->second;
			}
			return NULL;
		}

		inline SceneNode * getChildNodeRecursive(const string& name) 
		{
			if(m_Name == name)
				return this;

			std::map<string,SceneNode*>::iterator it = m_childNodes.find(name);
			if (it != m_childNodes.end())
			{
				return it->second;
			}
			else 
			{
				it = m_childNodes.begin();
				for(;it != m_childNodes.end();it++)
				{
					SceneNode * ret = it->second->getChildNodeRecursive(name);
					if(!ret)
						return ret;
				}
			}

			return NULL;
		}

		// detach all child nodes from this node.
		inline void detachAllNodes()
		{
			if(m_childNodes.empty()) return;
			m_childNodes.clear();
		}
		// traverse 
		inline NodeIterator getChildBegin()
		{
			assert(!m_childNodes.empty());
			return NodeIterator(m_childNodes.begin());
		}
		inline NodeIterator getChildEnd()
		{
			assert(!m_childNodes.empty());
			return NodeIterator(m_childNodes.end());
		}

		// local space
        inline void scale(Vector3 const& scal)
		{
			m_scale *= scal;
			m_scaleCurrent = false;
			if(m_parent)
				m_parent->notifyChildTransformChanged();
		
			thisNodeMoved();
		

		}
	
		inline void scale(float ix,float iy,float iz)
		{
			Vector3 temp = Vector3(ix,iy,iz);
			scale(temp);
		}
		

		inline void setScale(float ix,float iy,float iz){
			Vector3 temp = Vector3(ix,iy,iz);
			setScale(temp);
		}
		inline void setScale(Vector3  scal)
		{
			m_scale = scal;
			m_scaleCurrent =false;
			if(m_parent)
				m_parent->notifyChildTransformChanged();

			thisNodeMoved();
		}
        Vector3 const& getScale() const
		{
			if(m_scaleCurrent)
				return m_derivedScale;
			if(m_parent)
			{
				m_derivedScale =  this->m_scale * m_parent->getScale();
			}
			else
			{
				m_derivedScale = m_scale;
			}
			m_scaleCurrent = true;
			return m_derivedScale;
		}
		inline void translate(float ix,float iy,float iz)
		{
			translate(Vector3(ix,iy,iz));
		}
		inline void translate(const Vector3& trans)
		{
			m_translation += trans;
			if(m_parent)
				m_parent->notifyChildTransformChanged();
			m_transCurrent = false;
			// tell the attached movables that they have been moved.
			notifyMovablesForMove();
			notifyChildForMove();
			thisNodeMoved();
		}
		// set translation directly, in local space
		inline void setTranslation(float ix,float iy,float iz)
		{
			setTranslation(Vector3(ix,iy,iz));
		}
		inline void setTranslation(const Vector3& trans)
		{
			m_translation = trans;
			if(m_parent)
				m_parent->notifyChildTransformChanged();
			m_transCurrent = false;
			thisNodeMoved();
		}
		// translation in global space.
        Vector3 const& getTranslation() const
		{
			if(m_transCurrent)
				return m_derivedTranslation;
			if(m_parent)
			{
				m_derivedTranslation = m_parent->getOrientation()*this->m_translation*m_parent->getScale()
					+ m_parent->getTranslation();
			}
			else
			{
				m_derivedTranslation = m_translation;
			}
			m_transCurrent = true;
			return m_derivedTranslation;
		}
		inline const Vector3& getLocalTranslation() 
		{
			return m_translation;
		}
		inline const Vector3& getLocalScale()
		{
			return m_scale;
		}
		// in local space.
		inline void rotate(float ax,float ay,float az,float radian)
		{
			rotate(Vector3(ax,ay,az),radian);
		}
		void rotate(const Vector3& axis,float radian)
		{
			m_orientation = m_orientation*Quaternion(radian,axis);
			if(m_parent)
				m_parent->notifyChildTransformChanged();
			m_orientationCurrent = false;
			notifyMovablesForMove();
			notifyChildForMove();
			thisNodeMoved();
			
		}
		// get orientation in global space.
        Quaternion const& getOrientation() const
		{
			if(m_orientationCurrent)
				return m_derivedOrientation;
			if(m_parent)
			{
				 m_derivedOrientation = m_parent->getOrientation()*this->m_orientation;
			}
			else
			{
				m_derivedOrientation = m_orientation;
			}
			m_orientationCurrent = true;
			return m_derivedOrientation;
		}
		// set orientation directly in local space.
		inline void setOrientation(const Quaternion& quater)
		{
			m_orientation = quater;
			if (m_parent)
			{
				m_parent->notifyChildTransformChanged();
			}
			m_orientationCurrent = false;
			thisNodeMoved();
		}

        Matrix4 getWorldMatrix() const
		{
            if (WorldMatrixDirty) {
                WorldMatrix = Matrix4{getOrientation(), getScale(), getTranslation()};
                WorldMatrixDirty = false;
            }
			return WorldMatrix;
		}

		const BoundingBox& getBoundingBox();

		const BoundingBox& getGlobalBoudingBox()
		{
			if (m_globaBBox_current)
				return m_globalbbox;

			m_globalbbox = getBoundingBox();

			for (std::map<string, SceneNode *>::iterator itr = m_childNodes.begin(); itr != m_childNodes.end(); itr++)
			{
				m_globalbbox.merge(itr->second->getGlobalBoudingBox());
			}

			m_globaBBox_current = true;
			return m_globalbbox;
		}
		// when we test bounding box intersection 
		// we may first use this function.
		float getBoundingRadius();
		// make sure there is at least one entity.
        Point3 const& getBoundingCenter();

		const Sphere& getBoundingSphere();

		float getGlobalBoudingRadius();

		// make sure there is at least one entity.
        Point3 const& getGlobalBoundingCenter();

		const Sphere& getGlobalBoundingSphere();

		// scene element management
		inline void attachMovable( Movable* movable)
		{
			assert(movable);
			m_movables.insert(std::make_pair(movable->getName(),movable));
			movable->setParent(this);
			
		}

		void attachMovable( Entity* movable);
		// this operator is expensive.avoid using it.
		void detachMovable( const string &name);
		inline void detachMovable( Movable* movable)
		{
			assert(movable);
			detachMovable(movable->getName());
		}

		void detachAllMovables()
		{
			m_movables.clear();
			m_entities.clear();
		}

		Movable* getMovable( const string &name) const
		{
			std::map<string,Movable*>::const_iterator it = m_movables.find(name);
			if( it != m_movables.end())
				return it->second;
			return NULL;
		}
		// movable traversal
		MovableIterator getMovableBegin()
		{
			assert(!m_movables.empty());
			return MovableIterator(m_movables.begin());
		}

		MovableIterator getMovableEnd()
		{
			assert(!m_movables.empty());
			return MovableIterator(m_movables.end());
		}

		// event 
		// tell this node its children node has been moved.
		void notifyChildTransformChanged()
		{
			children_changed  = true;
			if(m_parent)
				m_parent->notifyChildTransformChanged();
		}

		bool getChildChanged()
		{
			return children_changed;
		}
		// tell its child node that their parent has been moved.
		void notifyChildForMove()
		{
			if(!m_childNodes.empty())
			{
				for( std::map<string,SceneNode*>::iterator it = m_childNodes.begin(); it != m_childNodes.end(); it++)
				{
					it->second->thisNodeMoved();
					
				}
			}
		}
		// tell this node its parent node has been moved.
		void notifyParentMoved()
		{
			m_transCurrent = false;
			m_orientationCurrent = false;
			m_scaleCurrent = false;
			m_BBoxCurrent = false;
			m_BSphereCurrent = false;
		}
		// tell attached movables that they have been moved.
		inline void notifyMovablesForMove()
		{
			if(!m_movables.empty())
			{
				for (std::map<string,Movable*>::iterator it = m_movables.begin(); it != m_movables.end(); it++)
				{
					it->second->notifyMoved();
				}
			}
		}
		void thisNodeMoved();

		SceneNode * getRoot()
		{
			if(m_parent)
				return m_parent->getRoot();
			return this;
		}

		// hint 
		inline bool hasChildNodes() const { return !m_childNodes.empty(); }
		inline bool hasMovables() const { return !m_movables.empty(); }
		inline bool hasEntities() const { return !m_entities.empty();}
		/// fast access to the entities list.
		const std::list<Entity*>& getEntities() const { return m_entities; }

		SceneManager* getCreator()
		{
			return m_creater;
		}

		void setEffect(Effect *effect)
		{
			for (std::list<Entity*>::iterator it = m_entities.begin(); it != m_entities.end(); ++it)
				(*it)->setEffect(effect);
			for (std::map<string, SceneNode*>::iterator it = m_childNodes.begin(); it != m_childNodes.end(); ++it)
				it->second->setEffect(effect);
		}

		void setCastShadow(bool cast)
		{
			m_bShadow = cast;
			for (std::list<Entity*>::iterator it = m_entities.begin(); it != m_entities.end(); ++it)
				(*it)->setCastShadow(cast);
		}
	
		bool isCastShadow()
		{
			return m_bShadow;
		}
		vector<SceneNode*> getAllChildNodes() {
			vector<SceneNode*> v;
			//for (auto& x : m_childNodes) {
			//	v.push_back(x.second);
			//	auto childs=x.second->getAllChildNodes();
			//}
			getAllChildRecur(this, v);
			return v;
		}
		void getAllChildRecur(SceneNode* node, vector<SceneNode*>& result) {
			if (node == NULL) return;
			for (auto& x : node->m_childNodes) {
				result.push_back(x.second);
				getAllChildRecur(x.second, result);		
			}
		}

		void AssignMask(unsigned int mask)
		{
			this->effectMask = mask;
			for (auto e : m_entities)
			{
				e->SetEffectMask(mask);
			}
		}

		unsigned int GetMask()
		{
			return this->effectMask;
		}

	private:

        mutable Matrix4 WorldMatrix{};
        mutable bool WorldMatrixDirty=true;
		Vector3 m_scale{1.0f};
        mutable Vector3 m_derivedScale{};
        mutable bool m_scaleCurrent{};

		// local translation
		Vector3 m_translation{};
		// derived translation
        mutable Vector3 m_derivedTranslation{};
		// hint whether the derived translation is current.
        mutable bool m_transCurrent{};
		Quaternion m_orientation{};
        mutable Quaternion m_derivedOrientation{};
		// hint whether the derived orientation is current
        mutable bool m_orientationCurrent{};
		string m_Name;
		BoundingBox m_boundingBox;
		// hint whether the derived BBox is current
		bool m_BBoxCurrent{};
		Sphere      m_boundingSphere;
		bool m_BSphereCurrent{};


		bool m_globaBBox_current{};
		BoundingBox m_globalbbox;

		bool m_globalSphere_current{};
		Sphere m_globalSphere;
		// stores all movables for querying and management.
		std::map<string,Movable*> m_movables;
		// only stores entities for rendering.
		std::list<Entity*> m_entities;
		//list for child nodes management
		std::map<string,SceneNode*> m_childNodes;
		
		SceneNode* m_parent{};

		bool children_changed{};
		bool m_bShadow{};
		unsigned int effectMask{};

		static NameGenerator* m_nameGenerator;
		
		SceneManager * m_creater{};
	};
}

