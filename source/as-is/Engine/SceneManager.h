#pragma once

#include "PreDefine.h"
#include "SceneNode.h"
#include "mathlib.h"
#include "light.h"
#include "RenderTarget.h"
#include "RenderItem.h"
#include <stack>
#include <map>
#include "ResourceManager.h"
#include "PreDefine.h"
#include <cereal/external/rapidjson/rapidjson.h>

using namespace std;
namespace HW
{
	/** forward class declaration
	*/


	enum  SkyTech
	{
		ST_SkyBox = 0x0,
		ST_SkyScatter = 0x1,
		ST_SkyTechNum
	};



	// RenderQueueItem

	//	class RenderTarget;
	/** scene manager :
	1. organize scene elements
	2. create/destroy scene elements
	3. maintain scene parameters. e.g. ambient light , background color.
	*/
	class SceneManager
	{
		friend class Camera;
	public:
		//-------------------common part-------------------------
		// never use this function.
		SceneManager(){}
		/** constructor with name
		*/
		SceneManager(const string& name, RenderSystem* renderer = NULL);
		//
		virtual ~SceneManager();
		/** attach a new renderSystem to this scene manager
		*/
		//	void NotifyRenderSystem(RenderSystem* renderer) { m_RenderSystem = renderer; }
		/** get render system
		*/
		RenderSystem* GetRenderSystem() const { return m_RenderSystem; }
		/** set name of this object
		*/
		void SetName(const string &_name) { name = _name; }
		/** return the name of this scene manager
		*/
		const string& GetName() const { return name; }
		/** get the scene root of this scene
		*/
		SceneNode* GetSceneRoot()  {
			if (!m_SceneRoot)
				m_SceneRoot = this->CreateSceneNodeImp("root");
			return m_SceneRoot;
		}
		SceneNode* LoadMeshtoSceneNode(const string&, const string&);
		//-----------------scene components creation part-----------------------

		/** create a new scene node named 'name'.
		scene manager will handle a reference to it
		for the convenience of querying.
		*/
		SceneNode* CreateSceneNode(const string &name);

		/** load scene from specific json
		*/
		void LoadSceneFromConfig(const string& path);

		/** the actual function for create scenenode
		*/
		virtual SceneNode * CreateSceneNodeImp(const string & name);
		/** factory method for creating light object
		*/
		Light* CreateLight(const string &name, Light::LightType type);
		/** factory method for create entity object
		*/
		Entity* CreateEntity(const string &name);
		Entity* CreateDynamicWeather(const string &name);
		/** factory method for create a camera object
		*/
		Camera* CreateCamera(const string &name);
		void addCamera(Camera* cam);
		// destroy . call these functions when
		// a specific object is detached from
		// a scene node. if the object to be destroyed
		// is not detached from its parent node
		// ,then it may cause an error.
		void destroy(Light* light);
		void destroy(Camera* camera);
		void destroy(Entity* entity);
		void destroy(SceneNode* node);
		// functions for querying.
		SceneNode* getSceneNode(const string& name) const;
		Light* getLight(const string &name);
		Camera* getCamera(const string& name);
		Entity* getEntity(const string& name);

		Sphere getSceneBoundingSphere();

		Sphere getSceneCastShadowSphere();

		BoundingBox getSceneBoundingBox();

		//create a rayscenenquery
		//RaySceneQuery * createRaySceneQuery();
		// fast access to the ligth list
		const std::map<string, Light*>&  getAllLightRefList()const
		{
			return m_lightRefList;
		}

		const std::map<string, Entity *>& getEntityList() const
		{
			return m_entityRefList;
		}
		std::map<string, RenderTarget *> & getShadowList()
		{
			return m_shadowRefList;
		}

		const std::map<string, vector<string>> & getEffectList() const
		{
			return this->effectList;
		}

		virtual void getVisibleRenderQueue(Camera * cam);

		virtual void getVisibleRenderQueue(Camera * cam, RenderQueue & renderQueue);

		virtual void pushNodeToQueue(string name, RenderQueue & renderQueue) const;
		virtual void getVisibleRenderQueue_as(Camera * cam, RenderQueue & renderQueue);
		void LoadScene(const string & scenefile_name, SceneNode* pRoot = NULL, bool castShadow = true);

		void printStatistic();

		// release the internal resource managered by the sceneManager
		// in this situation we mean the m_shadowRefList resource
		void releaseInternalRes();


	private:

		std::map<string, Entity*>  m_entityRefList;
		std::map<string, Light*>   m_lightRefList;
		std::map<string, RenderTarget *> m_shadowRefList;
		std::map<string, Camera*>  m_cameraRefList;
		std::map<string, RenderTarget *> m_rendertargetList;
		mutable std::map<string, vector<string>> effectList;

		void findVisibleNode(Camera * cam, SceneNode * node);

		void findVisibleNode(Camera *cam, SceneNode *node, RenderQueue &renderqueue);

	public:

		//---------------about shading--------------------------------------

		const Vector4& GetEnvAmbient() const { return m_EnvAmbient; }
		/** Set environment ambient intensity
		*/
		void SetEnvAmbient(const Vector4 &ambient) { m_EnvAmbient = ambient; }
		/** set the color for the background of the scene
		*/
		void SetBackgroundColor(const Vector4 &background) { m_BackgroundColor = background; }
		/** get background color of the scene
		*/
		const Vector4& GetBackgroundColor() const { return m_BackgroundColor; }

		virtual void addChangedNode(SceneNode * node);

		void addInfluenceCamera(Camera * cam);

		std::map<string, Entity *>& getChangedEntities()
		{
			return change_entity;
		}
		bool doneInfluence(Camera * came);

		inline bool CastShadow()
		{
			return mCastShadow;
		}

		inline void setCastShadow(bool shadow)
		{
			mCastShadow = shadow;
		}

		inline bool useGlobalEffect()
		{
			return m_useGlobalEffect;
		}
		inline void setUseGlobalEffect(bool global)
		{
			m_useGlobalEffect = global;
		}

		void clearGlobalEffect()
		{
			m_GlobalEffect = NULL;
			m_useGlobalEffect = false;
		}

		void setGlobalEffect(Effect * effect);
		Effect * getGlobalEffect();

		void setDepthEffect(Effect * effect);
		Effect * getDepthEffect();

		SceneNode * createSkyNode(string mname = "skysphere");
		void setUseSkyNode(bool on)
		{
			m_skyNodeEnable = on;
		}
		bool useSkyNode()
		{
			return m_skyNodeEnable;
		}


		Sky* CreateSky(SkyTech tech, float radius_scale = 1000.0);

		Sky * getSky();

		void setSky(Sky * sky)
		{
			if (sky != NULL)
				m_sky = sky;
		}
		Entity *  getSkyEntity();

		Entity * getScreeAlignedQuad();

		//Animation * getAnimation()
		//{
		//	if (!m_animation)
		//		m_animation = new Animation;
		//	return m_animation;
		//}
		//AnimationState * getAnimationState(void * pointer)
		//{
		//	return  getAnimation()->getAnimationState(pointer);
		//}
		/** Add Light To Active Light
		@param light  if light is null erase the index .else add light to index or tail of vector ;
		@param index  if index > m_active_light_list, push_to tail
		*/
		void setActiveLightVector(Light * light, int index);

		const vector<Light *> & getActiveLightVetcor() const;
		const vector<Light *> & getShadowLightVetcor() const;
		vector<Light *>  m_has_shadow_light_list;
		void updateShadowParam(Pass* pass) const;
		void updateLightStatus();
		rapidjson::Document scene_json_document;
		//vector<LightCameraParam> lightList;
		//map<string, vector<string>> caster;
		string name;
	protected:
		/**
		*/

		/** the root node of a scene which managed by the manager
		*/
		SceneNode * m_SceneRoot{};
		/** pointer to the RenderSystem to be used to render the scene
		*/
		RenderSystem *m_RenderSystem{};
		/** ambient intensity of the environment
		*/
		Vector4 m_EnvAmbient;
		/** back ground color used when rendering the scene
		*/
		Vector4 m_BackgroundColor;
		/** Scene cast shadow or not
		*/
		bool mCastShadow{true};
		// change list
		std::map<string, Entity *> change_entity;
		std::map<string, Camera *> influence_camera;

		// maintain a scene node list for querying.
		std::map<string, SceneNode*> m_sceneNodeRefList;

		// animation of this scenemanager;
		//Animation * m_animation;

		bool m_skyNodeEnable{};
		SceneNode * m_skyNode{};

		Sky * m_sky{};

		Entity * m_ScreenAlignedQuad{};

		Effect * m_depthEffect{};
		bool m_useGlobalEffect{};
		Effect * m_GlobalEffect{};

		/* Light Vector we assume that the maximum light is 4
		*/
		vector<Light *>  m_active_light_list;
	};
}

class SceneContainer :public ResourceManager<SceneManager*>{
public:
	static SceneContainer& getInstance()
	{
		static SceneContainer    instance;
		return instance;
	}


private:
	SceneContainer();
	SceneContainer(SceneContainer const&);              // Don't Implement.
	void operator=(SceneContainer const&); // Don't implement
	NameGenerator* namegen;

};

