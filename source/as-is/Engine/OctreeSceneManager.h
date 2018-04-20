#ifndef HW_OCTREESCENEMANAGER_H
#define HW_OCTREESCENEMANAGER_H

#include "SceneManager.h"

namespace HW
{
	class Octree;
	class OctreeNode;
	class OctreeSceneManager : public SceneManager
	{
	public:
		OctreeSceneManager(const string & name,RenderSystem * render = NULL);
		// the implemetation of create octreenode
		SceneNode * CreateSceneNodeImp(const string & name);
		// add octree node in octree structure
		void addOctreeNode(OctreeNode * node,Octree * octree,int curr_depth =0 );
		// build Octree
		void BuildOctree();
		// update the node when this node is moved
		virtual void addChangedNode(SceneNode * node);

		// the octree version of get renderque
		virtual void getVisibleRenderQueue(Camera * cam);

		// the octree version of new get renderQueue;
		virtual void getVisibleRenderQueue(Camera * cam,RenderQueue & renderQueue);

		// remove the node 
		void removeOctreeNode(OctreeNode * node);

		~OctreeSceneManager();
	private:
		// the root tree of the scene;
		Octree * m_Octree;
		// the max depth of the tree
		int m_Maxdepth;
		// flag of rebuild tree
		bool rebuild_tree;
		// travese octree and add octreenode into camera's render queue
		void traverseOctree(Octree * tree, Camera * cam);

		void traverseOctree(Octree *tree,Camera *cam,RenderQueue & renderqueue);

	};
}
#endif