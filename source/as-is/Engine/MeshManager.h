#pragma once
#include "OMesh.h"
#include "EngineUtil.h"
#include<string>

#include <map>

namespace FileLoader
{
	class Material;
}

namespace HW
{
		//mesh manager
		//1. load mesh from file.
		//2. maintain a mesh list shared among the framework.
		//3. query mesh by name.


	class Texture;
	class TextureFactory;
	class Renderable;
	class MeshManager
	{
	public:
		~MeshManager();
		/** singleton pattern
		*/


		static MeshManager& getSingleton();
		static MeshManager* getSingletonPtr();

		/** initialize this manager
		*/
		void initialize(TextureFactory * texturefactory);

		/** set the directory path for mesh files.*/
		void setMeshDirectory(const string& path) { m_meshDirectory = path; }
		/** set directory path for image files*/
		void setImageDirectory(const string& path) { m_imageDirectory = path; }

		MeshPtr loadMesh(const string& name,const string& filename,bool pathcontained = true);
		MeshPtr loadMesh_assimp(const string& name, path_util::Sanitized_Runtime_Path const& filename);
		MeshPtr loadMesh_assimp_check(const string& name, const string& filename);
		/** register the mesh to the system and it will be reused.
		*/
		void registerMesh(MeshPtr mesh);
		/** unregister mesh from the system.
		*/
		void unregisterMesh(const string& name);
		void unregisterMesh(MeshPtr mesh);
		// unregister all mesh from the framework.
		void clearMeshes();
		/** query mesh by name.
		*/
		MeshPtr getMeshByName(const string &name);

		void printStatistic();

		void releaseInternalRes();

	private:
		/** default constructor
		*/
		MeshManager();
		/** a static pointer to the unique mesh manager instance.
		*/
		static MeshManager* m_meshMgr;

		std::map<string,MeshPtr> m_MeshList;

		// path
		string m_meshDirectory;
		string m_imageDirectory;
	};
}

