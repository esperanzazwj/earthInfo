#include "MeshManager.h"
#include "ElemIterator.h"
#include "OMesh.h"
#include "SubMesh.h"
#include "Texture.h"
#include "engine_struct.h"
#include "assimp_loader.h"
#include "ResourceManager.h"
#include "TextureManager.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace HW
{
	MeshManager* MeshManager::m_meshMgr = NULL;

	//---------------------------------------------------------------
	MeshManager::MeshManager(){
	}

	MeshManager* MeshManager::getSingletonPtr()
	{
		if(!m_meshMgr)
			m_meshMgr = new MeshManager();
		return m_meshMgr;
	}

	MeshManager& MeshManager::getSingleton()
	{
		return *getSingletonPtr();
	}

	MeshPtr  MeshManager::loadMesh(const string& /*name*/,const string& /*filename*/,bool /*pathcontained*/ /* = true */)
	{
		return MeshPtr(NULL);
	}

		HW::MeshPtr MeshManager::loadMesh_assimp(const string& name, path_util::Sanitized_Runtime_Path const& filepath)
		{
			vector<as_Geometry*> geos;
			vector<as_Skeleton*> skes;
			vector<as_SkeletonAnimation*> anis;
			vector<as_Material*> mats;
			vector<as_Mesh*> meshs;
			Vector3 sMax, sMin;
			auto& tm = TextureManager::getInstance();
			Import3DFromFile(filepath, meshs, geos, skes, anis, mats, sMax, sMin);

			OMesh *mesh = new SubMesh(name);
			auto& g = GlobalResourceManager::getInstance();
			for (auto& x : geos) {
				g.as_geometryManager.add(x);
			}
			for (auto& x : skes) g.as_skeletonManager.add(x);
			for (auto& x : anis) g.as_skeletonAniManager.add(x);
			for (auto& x : mats) {
				g.as_materialManager.add(x);
				for (auto& t : x->textures){
					auto tex = t.second;
					g.as_textureManager.add(&tex);
					tm.CreateTextureFromImage(tex.path);
				}

			}
			for (auto& x : meshs){
				mesh->addSubMesh_as(x);
				g.as_meshManager.add(x);

			}

			mesh->m_modelBoundingBox = BoundingBox{sMax, sMin};
			mesh->m_modelSphere.center = (sMax + sMin)*0.5f;
			mesh->m_modelSphere.radius = (sMax - sMin).length()*0.5f;
			mesh->m_boundingBoxCurrent = true;
			mesh->m_boundingSphereCurrent = true;

			return MeshPtr(mesh);
		}

		HW::MeshPtr MeshManager::loadMesh_assimp_check(const string& name, const string& filename)
		{
			if (auto mesh = getMeshByName(name)) return mesh;

			auto path = path_util::Sanitized_Runtime_Path{filename};
			path_util::Working_Directory_File_Guard wdfg{filename};
			auto mesh = loadMesh_assimp(name, path);
			registerMesh(mesh);
			return mesh;
		}

		MeshManager::~MeshManager()
		{

		}

		void MeshManager::releaseInternalRes()
		{
			std::map<string,MeshPtr>::iterator itr_mesh = m_MeshList.begin();
			for(;itr_mesh != m_MeshList.end();itr_mesh++)
			{
				// do invalid mesh's egl resource
				itr_mesh->second->releaseInternalRes();
			}
		}

		void MeshManager::initialize(TextureFactory * /*texturefactory*/)
		{
		}

		void MeshManager::registerMesh(MeshPtr mesh)
		{
			std::pair<std::map<string, MeshPtr>::iterator, bool> ret;
			ret = m_MeshList.insert(std::make_pair(mesh->getName(), mesh));
			if (ret.second)
			{
				//printf("Mesh %s insert success",mesh->getName().c_str());
			}
			else
				printf("Mesh %s Insert Failed", mesh->getName().c_str());
		}

		void MeshManager::unregisterMesh(const string& name)
		{
			std::map<string, MeshPtr>::iterator it = m_MeshList.find(name);
			if (it != m_MeshList.end())
				m_MeshList.erase(it);
		}

		void MeshManager::unregisterMesh(MeshPtr mesh)
		{
            assert(mesh && "Trying to unregister a null mesh pointer"); // report a error
			unregisterMesh(mesh->getName());
		}

		void MeshManager::clearMeshes()
		{
			if (m_MeshList.empty())return;
			m_MeshList.clear();
		}

		HW::MeshPtr MeshManager::getMeshByName(const string &name)
		{
			std::map<string, MeshPtr>::iterator it = m_MeshList.find(name);
			if (it != m_MeshList.end())
				return it->second;
			return MeshPtr(0);
		}
}

