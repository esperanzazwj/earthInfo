#ifndef HW_OCTREE_H
#define HW_OCTREE_H
#include "mathlib.h"
#include <string>
#include <map>

namespace HW
{
	//the node attached on octree structure
	class OctreeNode;
	class Octree
	{
	public:
		Octree(Octree * parent = NULL);
		const BoundingBox &  getBBox() 
		{
			return m_bbox;
		}
		void setBoundingBox(BoundingBox & bbox)
		{
			m_bbox = bbox;
		}
		unsigned int getNumSceneNode()
		{
			return m_numSceneNode;
		}
		// attach octnode on this octree and increase the num of scenenode of 
		// this tree and its parents
		void attachOctreeNode(OctreeNode * octnode);

		// dettach octnode on this octree and decrease the num of scenenode of 
		// this tree and its parents
		void removeOctreeNode(OctreeNode * octnode);

		// to see if which children tree should this octnode be store
		// return the pointer if this node is not suitable for store in the 
		// children return  NULL
		Octree* getChildOctree(OctreeNode * octnode);

		//use the index to get the child tree
		Octree * getChildOctree(int x,int y,int z)
		{
			return m_chlidren[x][y][z];
		}

		// see if the octreeNode is fitful for this octree
		bool contain(OctreeNode * octnode);

		// return the node attached on this tree
		auto& getNodeList() const { return m_nodelist; }

		void increaseNodeNum()
		{
			m_numSceneNode++;
			if(m_parent)
				m_parent->increaseNodeNum();
		}

		void decreaseNodeNum()
		{
			m_numSceneNode--;
			if(m_parent)
				m_parent->decreaseNodeNum();
		}
		~Octree();
	private:
		//parent of this octree
		Octree * m_parent;
		// Bounding Box of this Octree represent
		BoundingBox m_bbox;
		// num of octreenode attached on this tree and his children;
		unsigned int m_numSceneNode;
		// its eight children;
		Octree * m_chlidren[2][2][2];
		// the octreenode that attached on this tree 
		// shoud be release by sceneManager
		std::map<std::string, OctreeNode*> m_nodelist;
	};
}
#endif
