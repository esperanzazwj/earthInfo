#pragma once
#include "mathlib.h"
#include<string>
#include "ElemIterator.h"
#include <map>
#include <vector>
#include "engine_struct.h"


namespace HW
{
	/** 
	*/
	class MeshManager;
	/** a wrapper for sub meshes.
		1. management sub meshes.
		2. organize the mesh 
		3. manage bounding box and bounding sphere.
	*/
	class OMesh
	{

		friend class MeshManager;
	public:
		//typedef mapIterator<String,SubMesh*> SubMeshIterator;
		//typedef mapIterator<String,Renderable *> RenderableIterator;

		OMesh() : m_boundingBoxCurrent(false),m_boundingSphereCurrent(false){}

		OMesh(const string &name);

		virtual ~OMesh();

		void setName(const string &name) { m_Name = name; }

		const string& getName()const { return m_Name; }


		//virtual void addSubMesh(Renderable* submesh)
		//{

		//}
		
		void deleteSubMesh(const string & /*name*/)
		{

		}

		void  deleteAllSubMeshes()
		{
		}

		void setWireFrame(bool set = true);
		/** return the bounding box in model space
		*/
		BoundingBox& getModelBoundingBox();
		/** 
		*/
		Sphere& getModelBoundingSphere();
		// set render effect
		void setLocalEffect(Effect * effect);

		void releaseInternalRes();

		virtual void addSubMesh_as(as_Mesh* submesh)
		{
			assert(submesh);
			m_SubMeshList_as[submesh->name]=submesh;
			m_boundingSphereCurrent = false;
			m_boundingBoxCurrent = false;
		}
		std::map<string, as_Mesh*>	m_SubMeshList_as;
	protected:


		string m_Name;
		/** bounding box in model space.
		*/
		BoundingBox m_modelBoundingBox;
		bool m_boundingBoxCurrent;
		/** bounding sphere in model space.
		*/
		Sphere m_modelSphere;
		bool m_boundingSphereCurrent;

		
	};

	/** shared pointer class*/
	typedef OMesh* MeshPtr;
	
}
