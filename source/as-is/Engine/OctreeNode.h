#ifndef HW_OCTREENODE_H
#define HW_OCTREENODE_H
#include "SceneNode.h"
#include<string>
namespace HW
{
	// octree node is a scenenode attached on an octree 
	// it is a subclass of scenenode,so it has tranformation
	// and entity attached on itself ,  scenenode  has 
	// children node ,but in octreenode ,it's children node 
	// should not be used

	class Octree;
	class SceneManager;

	class OctreeNode : public SceneNode
	{
	public:
		OctreeNode() = default;
		OctreeNode(const string & m_name, SceneManager * creator) : SceneNode{m_name, creator} {}

		void setOctree(Octree * oct) { m_octree = oct; }
		Octree * getOctree() { return m_octree; }

	private:
		Octree * m_octree{};
	};
}
#endif
