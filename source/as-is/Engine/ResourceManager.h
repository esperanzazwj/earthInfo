#pragma once
#include "engine_struct.h"
#include "PreDefine.h"
#include <iostream>

//ResourceManager: for pointers of animation,mesh,geometry and so on
// for objects which  has a name interface,of course we assume they don't has same name 
template <typename T>
class ResourceManager
{
public:
	map<string, T> resources;
	
	T get(string name){
		auto it = resources.find(name);
		if (it != resources.end())
			return it->second;
		else {
			return NULL;
		}
	}

	void add(T m){
		resources[m->name] = m;

	}
	void rename(T /*m*/,string /*name*/){

	}
	void remove(string name){
		auto it = resources.find(name);
		if (it != resources.end()){
			//delete it->second;
			resources.erase(it);
		}
	}
};

//单例全局资源管理器
class GlobalResourceManager
{
public:
	static GlobalResourceManager& getInstance()
	{
		static GlobalResourceManager    instance;
		return instance;
	}

	ResourceManager<as_Mesh*> as_meshManager;
	ResourceManager<as_Geometry*> as_geometryManager;
	ResourceManager<as_Skeleton*> as_skeletonManager;
	ResourceManager<as_SkeletonAnimation*> as_skeletonAniManager;
	ResourceManager<as_Material*> as_materialManager;
	ResourceManager<as_TexturePath*>	as_textureManager;


	RenderSystem* m_RenderSystem;
	RenderStrategy* m_RenderStrategy;
	PerObjectUniformGetter* m_PerObjectUniformGetter;
	UniformSetter* m_UniformSetter;
	TextureFactory* m_TextureFactory;
	GpuProgramFactory* m_GpuProgramFactory;
	GeometryFactory* m_GeometryFactory;
	RenderTargetFactory* m_RenderTargetFactory;
	
	SamplerFactory* m_SamplerFactory;
	string m_platform_info;
private:
	GlobalResourceManager() {}
	GlobalResourceManager(GlobalResourceManager const&);              // Don't Implement.
	void operator=(GlobalResourceManager const&); // Don't implement
};
