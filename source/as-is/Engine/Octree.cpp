#include "Octree.h"
#include "OctreeNode.h"
namespace HW
{
	Octree::Octree(Octree * parent):
	m_parent(parent),m_bbox(Vector3::ZERO,Vector3::ZERO),m_numSceneNode(0)
	{
		for(unsigned int i = 0 ;i < 2; i++)
			for(unsigned int  j = 0; j < 2 ;j++)
				for(unsigned int k = 0; k < 2;k++)
					m_chlidren[i][j][k] = NULL;
		
	}

	Octree::~Octree()
	{
		m_numSceneNode = 0;

		for(unsigned int i = 0 ;i < 2; i++)
			for(unsigned int  j = 0; j < 2 ;j++)
				for(unsigned int k = 0; k < 2;k++)
				{
					delete m_chlidren[i][j][k];
					m_chlidren[i][j][k] = NULL;
				}
	}

	void Octree::attachOctreeNode(OctreeNode * octnode)
	{
		
		m_nodelist[octnode->getName()] = octnode;
		octnode->setOctree(this);
		increaseNodeNum();
	}

	void Octree::removeOctreeNode(OctreeNode * octnode)
	{
		std::map<string,OctreeNode *>::iterator it = m_nodelist.find(octnode->getName());
		if(it != m_nodelist.end())
		{
			it->second->setOctree(NULL);
			m_nodelist.erase(it);
			decreaseNodeNum();
		}
	}

	Octree * Octree::getChildOctree(OctreeNode * octnode)
	{
		const BoundingBox & bbox = octnode->getBoundingBox();
		if(bbox.isNull())
			return NULL;
		Vector3 node_size = bbox.getSize();
		Vector3 tree_half  = m_bbox.getHalfSize();
		// pay atention to the operator < return true 
		//only when every member of node_size is less tahn tree_half
		if(!(node_size < tree_half))
			return NULL;
			
		const Vector3 & node_center = bbox.getCenter();
		const Vector3 & tree_center = m_bbox.getCenter();
		Vector3 tree_min = m_bbox.min_point,tree_max = m_bbox.max_point; 
		int x,y ,z;
		if(node_center.x < tree_center.x)
			x = 0;
		else
			x =1;

		if(node_center.y < tree_center.y)
			y = 0;
		else
			y =1;

		if(node_center.z < tree_center.z)
			z = 0;
		else
			z =1;

		if(m_chlidren[x][y][z] == NULL)
		{
			Octree * new_tree = m_chlidren[x][y][z] = new Octree(this);
			Vector3 new_min,new_max;
			if(x == 0)
			{
				new_min.x = tree_min.x;
				new_max.x = tree_center.x;
			}else
			{
				new_min.x = tree_center.x;
				new_max.x = tree_max.x;
			}

			if(y == 0)
			{
				new_min.y = tree_min.y;
				new_max.y = tree_center.y;
			}else
			{
				new_min.y = tree_center.y;
				new_max.y = tree_max.y;
			}
			if(z == 0)
			{
				new_min.z = tree_min.z;
				new_max.z = tree_center.z;
			}else
			{
				new_min.z = tree_center.z;
				new_max.z = tree_max.z;
			}
			BoundingBox new_bbox(new_min,new_max);
			new_tree->setBoundingBox(new_bbox);	
		}


		//std::cout<<bbox<<m_bbox<<"["<<x<<","<<y<<","<<z<<"]"<<std::endl;

		return m_chlidren[x][y][z];

	}


	bool Octree::contain(OctreeNode * octnode)
	{
		const BoundingBox & nodebox = octnode->getBoundingBox();
		return m_bbox.contains(nodebox);
	}
}
