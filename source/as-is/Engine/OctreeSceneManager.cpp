#include "OctreeSceneManager.h"
#include "Camera.h"
#include "Entity.h"
#include "Octree.h" 
#include "OctreeNode.h"
 unsigned int intersect_num = 0;
namespace HW
{
	OctreeSceneManager::OctreeSceneManager(const string & name,RenderSystem * render /* = NULL */):
	SceneManager(name,render)
	{
		m_Octree = new Octree(NULL);
		BoundingBox bbox(Vector3(-1000,-1000,-1000),Vector3(1000,1000,1000));
		m_Octree->setBoundingBox(bbox);
		m_Maxdepth = 5;
		rebuild_tree = false;
	}

	SceneNode * OctreeSceneManager::CreateSceneNodeImp(const string & name)
	{
		auto node = new OctreeNode(name, this);
		m_sceneNodeRefList[name] = node;
		return node;
	}

	void OctreeSceneManager::addOctreeNode(OctreeNode * node,Octree * octree,int curr_depth /* =0 */ )
	{
		if(octree == NULL)
			return ;
		Octree * child_tree = octree->getChildOctree(node);
		if(curr_depth < m_Maxdepth && child_tree != NULL)
		{
			addOctreeNode(node,child_tree,++curr_depth);
		}
		else
		{
		//	std::cout<<node->getName()<<" "<<curr_depth<<std::endl;
			octree->attachOctreeNode(node);
		}
	}

	void OctreeSceneManager::BuildOctree()
	{
		BoundingBox bbox;
		std::map<string,SceneNode*>::iterator it_scenenode = m_sceneNodeRefList.begin();
		for(;it_scenenode != m_sceneNodeRefList.end();it_scenenode ++)
		{
			if(m_SceneRoot == it_scenenode->second->getRoot())
			{
				const BoundingBox & node_bbox = it_scenenode->second->getBoundingBox();
				bbox.merge(node_bbox);
			}
		}
		m_Octree->setBoundingBox(bbox);
		std::cout<<bbox;
		for(it_scenenode = m_sceneNodeRefList.begin(); it_scenenode != m_sceneNodeRefList.end();it_scenenode++)
		{
			if(m_SceneRoot == it_scenenode->second->getRoot())
			{
				OctreeNode * octnode = static_cast<OctreeNode *>(it_scenenode->second);
				addOctreeNode(octnode,m_Octree);
			}
		}

		rebuild_tree = true;

	}

	void OctreeSceneManager::addChangedNode(SceneNode * scenennode)
	{

		if(m_SceneRoot != scenennode->getRoot())
			return;

		// use the same way to process changed node ,the only difference is to 
		// ignore the node's children nodes;
		if(!influence_camera.empty())
		{
			influence_camera.clear();
			change_entity.clear();
		}

		const std::list<Entity *>& entity_list = scenennode->getEntities();
		for (std::list<Entity *>::const_iterator move_it = entity_list.begin();move_it != entity_list.end();move_it ++)
		{
			//std::map<String,Entity *>::iterator it = change_entity.find((*move_it)->getName());
			//if(change_entity.end() != it)
			//	return;

			// use the operator of [] to add or replace the entity;
			Entity * entity = (*move_it);
			change_entity[entity->getName()] = entity;
		}


		// only when the octree structure is constructed ,we update these changes into octree structure
		if(rebuild_tree)
		{
			OctreeNode * node = static_cast<OctreeNode *>(scenennode);
			Octree * oct = node->getOctree();
			if(oct == NULL)
			{
				// when this node is out of whole tree,intert into root tree
				if(!m_Octree->contain(node))
					m_Octree->attachOctreeNode(node);
				else
					addOctreeNode(node,m_Octree);
				return;
			}

			// the current parent tree cannot holds this node
			if(!oct->contain(node))
			{
				oct->removeOctreeNode(node);
				if(!m_Octree->contain(node))
					m_Octree->attachOctreeNode(node);
				else
					addOctreeNode(node,m_Octree);

			}
		}

		


	}

	void OctreeSceneManager::traverseOctree(Octree * tree, Camera * cam)
	{
		
		if(tree->getNumSceneNode() == 0)
			return;
		BoundingBox bbox = tree->getBBox();
		Vector3 half_size = bbox.getHalfSize();
		bbox.setExtents(bbox.max_point + half_size ,bbox.min_point - half_size);
		if (auto visibility = cam->intersects(bbox))
		{
			bool insert = false;
			const std::map<string,OctreeNode *> & nodelist  = tree->getNodeList();
			std::map<string,OctreeNode *>::const_iterator it_node = nodelist.begin();
			for(;it_node != nodelist.end(); it_node++)
			{
				insert = false;
				if(it_node->second->hasEntities())
				{
					if(visibility == 1)
						insert = true;
					else if(visibility == 2)
					{
						insert = cam->intersects(it_node->second->getBoundingBox());
						intersect_num ++;
					}
					if(insert)
					{
						
						const std::list<Entity*>& entities = it_node->second->getEntities();
						for (std::list<Entity*>::const_iterator it = entities.begin(); it != entities.end(); it++)
						{
							// test if this entity is visible
							if(visibility == 2)
								cam->insertMoveableToRenderqueue(*it);
							else
								cam->insertVisibleMoveable(*it);
							

						}
					}
					
				}
			}
			intersect_num ++;

			for(unsigned int x = 0 ;x < 2;x ++)
				for(unsigned int y = 0; y < 2; y++)
					for(unsigned int z = 0; z < 2; z++)
					{
						Octree * child = tree->getChildOctree(x,y,z);
						if(child)
							traverseOctree(child,cam);
					}

		}
		intersect_num++;

		// some code for debug use 
		/*else
		{
			bbox = tree->getBBox();
			Vector3 half_size = bbox.getHalfSize();
			bbox.setExtents(bbox.max_point + half_size ,bbox.min_point - half_size);

			cam->intersects(bbox);
			Vector3 m_min = bbox.min_point,m_max = bbox.max_point;
			std::cout<<"vector3 "<<m_min.x<<" "<<m_min.y<<" "<<m_min.z<<std::endl;
			std::cout<<"vector3 "<<m_max.x<<" "<<m_max.y<<" "<<m_max.z<<std::endl;
			m_max = bbox.getSize();
			std::cout<<"size "<<m_max.x<<" "<<m_max.y<<" "<<m_max.z<<std::endl;
			const std::map<String,OctreeNode *> & nodelist  = tree->getNodeList();
			std::map<String,OctreeNode *>::const_iterator it_node = nodelist.begin();
			for(;it_node != nodelist.end(); it_node++)
			{
				m_min = it_node->second->getBoundingBox().min_point;
				m_max = it_node->second->getBoundingBox().max_point;
				m_max = it_node->second->getBoundingBox().getSize();
				std::cout<<"vector3 "<<m_min.x<<" "<<m_min.y<<" "<<m_min.z<<std::endl;
				std::cout<<"vector3 "<<m_max.x<<" "<<m_max.y<<" "<<m_max.z<<std::endl;
				std::cout<<"size "<<m_max.x<<" "<<m_max.y<<" "<<m_max.z<<std::endl;
				std::cout<<cam->intersects(it_node->second->getBoundingBox());
			}
		}*/


	}

	void OctreeSceneManager::traverseOctree(Octree *tree,Camera *cam,RenderQueue & render_queue)
	{
		if(tree->getNumSceneNode() == 0)
			return;
		BoundingBox bbox = tree->getBBox();
		Vector3 half_size = bbox.getHalfSize();
		bbox.setExtents(bbox.max_point + half_size ,bbox.min_point - half_size);

		if (auto visibility = cam->intersects(bbox))
		{
			const std::map<string,OctreeNode *> & nodelist  = tree->getNodeList();
			std::map<string,OctreeNode *>::const_iterator it_node = nodelist.begin();
			for(;it_node != nodelist.end();it_node++)
			{
				if(visibility == 2 && cam->intersects(it_node->second->getBoundingBox()) == 0)
					continue;
				const std::list<Entity*>& entities = it_node->second->getEntities();
				for (std::list<Entity*>::const_iterator it = entities.begin(); it != entities.end(); it++)
				{
					(*it)->updateRenderqueue_as(cam,render_queue);
					
				}
			}

			for(unsigned int x = 0 ;x < 2;x ++)
				for(unsigned int y = 0; y < 2; y++)
					for(unsigned int z = 0; z < 2; z++)
					{
						Octree * child = tree->getChildOctree(x,y,z);
						if(child)
							traverseOctree(child,cam,render_queue);
					}
		}

	}

	void OctreeSceneManager::getVisibleRenderQueue(Camera * cam)
	{
		//printf("enter octree getrenderque %d ,%d",rebuild_tree,cam->renderQueueEstablished());
		if(!rebuild_tree)
			BuildOctree();
		if(!cam->renderQueueEstablished())
			traverseOctree(m_Octree,cam);
		else
		{
			if(!change_entity.empty() && !this->doneInfluence(cam))
			{
				std::map<string,Entity*>::iterator entity_it = change_entity.begin();
				for(;entity_it != change_entity.end();entity_it++)
				{
					cam->insertMoveableToRenderqueue(entity_it->second);
				 
				}

				
			}
		}
	//	printf("complete construct");
		addInfluenceCamera(cam);
		cam->establishRenderQueue(true);
		std::cout<<cam->getName()<<" intersects "<<cam->intersect_num <<std::endl;
		intersect_num = 0;
	}

	void OctreeSceneManager::getVisibleRenderQueue(Camera * cam,RenderQueue & renderQueue)
	{

		if(!rebuild_tree)
			BuildOctree();
		traverseOctree(m_Octree,cam,renderQueue);

	}

	OctreeSceneManager::~OctreeSceneManager(){
		delete m_Octree;
	}
}
