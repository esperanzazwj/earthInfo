#include "SceneNode.h"
#include "SceneManager.h"
#include "Entity.h"
#include "../../util/unreachable.macro.hpp"

namespace HW
{
	NameGenerator* SceneNode::m_nameGenerator = new NameGenerator("SceneNode");

	SceneNode::SceneNode() : m_Name{m_nameGenerator->Generate()}
	{}

	SceneNode::SceneNode(std::string const& name, SceneManager* creater)
		: m_Name(name.empty() ? m_nameGenerator->Generate() : name)
        , m_creater(creater)
	{}

	void SceneNode::detachMovable(const string &name)
	{
		std::map<string,Movable*>::iterator it = m_movables.find(name);
		if( it != m_movables.end())
		{
			Movable::MovableType type = it->second->getMovableType();
			if(type == Movable::MT_ENTITY)
			{
				for (std::list<Entity*>::iterator iter = m_entities.begin(); iter!=m_entities.end(); iter++)
				{
					if(!(*iter)->getName().compare( name))
					{
						m_entities.erase(iter);
					}
				}
			}
			m_movables.erase(it);
		}
	}

	void SceneNode::thisNodeMoved()
	{
		WorldMatrixDirty = true;
		m_transCurrent = false;
		m_scaleCurrent = false;
		m_orientationCurrent = false;
		m_BBoxCurrent = false;
		m_BSphereCurrent = false;
		notifyChildForMove();

		notifyMovablesForMove();

		m_creater->addChangedNode(this);

	}

	const BoundingBox& SceneNode::getBoundingBox()
	{
		if(m_BBoxCurrent)
			return m_boundingBox;
		// merge bounding box of the movables

		if(!m_entities.empty())
		{
			std::list<Entity*>::iterator it = m_entities.begin();
			m_boundingBox = (*it)->getBoundingBox();
			it++;

			while( it != m_entities.end())
			{
				m_boundingBox.merge((*it)->getBoundingBox());
				it++;
			}
		}
		else
		{
			m_boundingBox.setNull();
		}
		m_BBoxCurrent = true;
		return m_boundingBox;
	}

	float SceneNode::getBoundingRadius()
	{
		if(m_BSphereCurrent)
			return m_boundingSphere.radius;
		if( !m_entities.empty())
		{
			getBoundingSphere();
			return m_boundingSphere.radius;
		}
		else
		{
			return 0;
		}
	}

	float SceneNode::getGlobalBoudingRadius()
	{
		if (m_globalSphere_current)
			return m_globalSphere.radius;
		if (!m_entities.empty() ||!m_childNodes.empty())
		{
			m_globalSphere = getBoundingSphere();
			std::map<string, SceneNode *>::iterator it = m_childNodes.begin();
			while (it != m_childNodes.end())
			{
				m_boundingSphere.merge((it->second)->getGlobalBoundingSphere());
				it++;
			}
			m_globalSphere_current = true;
			return m_globalSphere.radius;
		}
		else
		{
			return 0;
		}
	}

	const Sphere& SceneNode::getBoundingSphere()
	{
		if(m_BSphereCurrent)
			return m_boundingSphere;
		if( !m_entities.empty())
		{
			std::list<Entity*>::iterator it = m_entities.begin();
			m_boundingSphere = (*it)->getBoundingSphere();
			it++;
			while( it != m_entities.end())
			{
				m_boundingSphere.merge((*it)->getBoundingSphere());
				it ++;
			}
			m_BSphereCurrent = true;
			return m_boundingSphere;
		}
		else
		{
			return m_boundingSphere;
		}
	}

	const Sphere& SceneNode::getGlobalBoundingSphere()
	{
		if (m_globalSphere_current)
			return m_globalSphere;
		if (!m_entities.empty() || !m_childNodes.empty())
		{

			m_globalSphere = getBoundingSphere();
			std::map<string, SceneNode *>::iterator it = m_childNodes.begin();
			while (it != m_childNodes.end())
			{
				m_boundingSphere.merge((it->second)->getGlobalBoundingSphere());
				it++;
			}
			m_globalSphere_current = true;
			return m_globalSphere;
		}
		else
		{
			return m_globalSphere;
		}
	}

    Point3 const& SceneNode::getBoundingCenter()
	{
		if(m_BSphereCurrent)
			return m_boundingSphere.center;

        if (!m_entities.empty()) {
			getBoundingSphere();
			return m_boundingSphere.center;
		}

        SS_UTIL_UNREACHABLE();
    }

    Point3 const& SceneNode::getGlobalBoundingCenter()
	{
		if (m_globalSphere_current)
			return m_globalSphere.center;

        if (!m_entities.empty() || !m_childNodes.empty()) {
			m_globalSphere = getBoundingSphere();
            for (auto& entry: m_childNodes) {
                m_globalSphere.merge(entry.second->getGlobalBoundingSphere());
			}
			return m_globalSphere.center;
		}

        SS_UTIL_UNREACHABLE();
	}

	void SceneNode::attachMovable( Entity* movable )
	{
		m_entities.push_back(movable);
		attachMovable(static_cast<Movable *>(movable));
	}

	//--------------------------------------------------------------------------

}

