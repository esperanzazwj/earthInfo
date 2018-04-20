#include "SceneManager.h"
#include "SceneNode.h"
#include "Entity.h"
#include "Camera.h"
#include "MeshManager.h"
#include "EngineUtil.h"
#include "RenderUtil.h"
#include "Pipeline/effectfactory.h"
#include <cassert>
#include <cereal/external/rapidjson/rapidjson.h>
#include <stdexcept>

#include "../../util/unreachable.macro.hpp"

namespace HW
{
	namespace
	{
		auto intersect = 0u;
		constexpr auto maximum_light_num = 80;
	}

	SceneManager::SceneManager(const string& name, RenderSystem * render)
		: name(name), m_RenderSystem(render)
	{}

	SceneNode* SceneManager::CreateSceneNode(const string &name)
	{
		auto SceneNodeit = m_sceneNodeRefList.find(name);
		if (SceneNodeit == m_sceneNodeRefList.end())
			return CreateSceneNodeImp(name);
		else
			return NULL;


		//// in some case the sceneNode export from 3dsmax has the samename;
		//// so we have to suppport them by rename it
		//SceneNode* node = NULL;
		//if (m_sceneNodeRefList.find(name) != m_sceneNodeRefList.end())
		//{
		//	node = CreateSceneNodeImp("");

		//}
		//else
		//	node = CreateSceneNodeImp(name);


		//// handle a reference by manager


		////	assert((m_sceneNodeRefList.find(name) == m_sceneNodeRefList.end())
		////		&& "scene node already exists.");

		//std::pair<std::map<string, SceneNode *>::iterator, bool > ret;
		//ret = m_sceneNodeRefList.insert(std::make_pair(name, node));
		//if (ret.second)
		//{
		//	//	printf("Node %s Insert Success",name.c_str());
		//}
		//else
		//	printf("Node %s Insert Failed", name.c_str());
		//return node;
	}


	ShadowTech GetShadowTech(string tech)
	{
		if (tech == "PCSS") return ShadowTech::PCSS;
		if (tech == "PCF") return ShadowTech::PCF;
		if (tech == "SM") return ShadowTech::NativeShadowMap;
		if (tech == "VSM") return ShadowTech::VSM;
		if (tech == "NONE") return ShadowTech::NO_Shadow;
		if (tech == "CSM") return ShadowTech::CSM;
        SS_UTIL_UNREACHABLE();
	}

	SceneNode* SceneManager::LoadMeshtoSceneNode(const string& pfile, const string& name) {
		auto meshMgr = MeshManager::getSingletonPtr();
		MeshPtr mesh = meshMgr->loadMesh_assimp_check(name, pfile);
		if (mesh == NULL) return NULL;
		Entity* entity = getEntity(name);
		if (entity == NULL)
			entity = CreateEntity(name);
		entity->setMesh(mesh);
		SceneNode* snode = getSceneNode(name);
		if (snode == NULL) {
			snode = CreateSceneNode(name);
			snode->attachMovable(entity);
		}
		return snode;
	}

	void SceneManager::LoadSceneFromConfig(const string& filename)
	{
		string file_content = file_util::slurp(filename);

		scene_json_document.Parse(file_content.c_str());
		const rapidjson::Value& root = scene_json_document["root"];
		const rapidjson::Value& objects = root["objects"];

		if(!objects.IsArray())
		throw runtime_error("objects is not an json array!");

		auto sceneRoot = this->GetSceneRoot();
		auto objectNode = CreateSceneNode("objectNode");
		auto lightNode = CreateSceneNode("lightNode");
		sceneRoot->attachNode(objectNode);
		sceneRoot->attachNode(lightNode);
		int lightSize = 0;

		// --- lights ---
		// --------------------------------------------
		try
		{
			if (!root.HasMember("lights"))
				throw std::runtime_error("No lights found in scene config.");

			const rapidjson::Value& lights = root["lights"];
			lightSize = lights.Size();
			for (int i = 0; i < lightSize; i++)
			{
				auto& light = lights[i];
				string name = "";
				string pos = "0,0,0";
				string color = "1,1,1";
				string tech = "";
				float fov = 45.0f;
				float near = 1.0f;
				float far = 100.0f;
				float radius = 0.1f;
				float bias = -0.01f;
				bool manualNearFar = false;
				int samplePattern = 0;

				if (light.HasMember("name"))
					name = light["name"].GetString();
				if (light.HasMember("position"))
					pos = light["position"].GetString();
				if (light.HasMember("color"))
					color = light["color"].GetString();
				if (light.HasMember("shadowTech"))
					tech = light["shadowTech"].GetString();
				if (light.HasMember("fov"))
					fov = light["fov"].GetFloat();
				if (light.HasMember("near"))
				{
					near = light["near"].GetFloat();
					manualNearFar = true;
				}
				if (light.HasMember("far"))
				{
					far = light["far"].GetFloat();
					manualNearFar = true;
				}
				if (light.HasMember("radius"))
					radius = light["radius"].GetFloat();
				if (light.HasMember("bias"))
					bias = light["bias"].GetFloat();
				if (light.HasMember("samplePattern"))
					samplePattern = light["samplePattern"].GetInt();

				auto p = Vector3{pos};
				auto c = Vector3{color};

				//this->lightList.push_back(LightCameraParam(p, c, fov));

				// --- new light struct ---
				Light* newLight = new Light();
				newLight->setName(name);
				newLight->setPosition(p);
				newLight->Diffuse = Vector4{c, 1.0f};
				newLight->fov = fov;
				newLight->near = near;
				newLight->far = far;
				newLight->radius = radius;
				newLight->bias = bias;
				newLight->manualNearFar = manualNearFar;
				newLight->tech = GetShadowTech(tech);
				newLight->samplePattern = samplePattern;
				this->m_has_shadow_light_list.push_back(newLight);

				// --- light scene node ---
				auto light_temp = LoadMeshtoSceneNode("model/wushi/wushi.obj", "light"+to_string(i));
				light_temp->setTranslation(p);
				light_temp->setScale(Vector3(0.1f));
				lightNode->attachNode(light_temp);
			}
		}
		catch (std::runtime_error const& e)
		{
			std::cerr << "!! WARNING: " << e.what() << "\n";
		}

		// --- objects ---
		// --------------------------------------------
		for (rapidjson::SizeType i = 0; i < objects.Size(); i++) // Uses SizeType instead of size_t
		{
			auto& obj = objects[i];
			string name = "";
			string path = "";
			auto effectMask = 0u;

			struct rotation_by_axis
			{
				Vector3 axis;
				float radian;

				rotation_by_axis(Vector3 axis, float radian)
					: axis{axis}, radian{radian}
				{}
			};

			auto trans = Vector3{0.0f, 0.0f, 0.0f};
			auto scale = Vector3{1.0f, 1.0f, 1.0f};
			std::vector<rotation_by_axis> rotations;

			if(obj.HasMember("name"))
				name = obj["name"].GetString();
			if(obj.HasMember("path"))
				path = obj["path"].GetString();
			if (obj.HasMember("effects"))
			{
				const rapidjson::Value& effects = obj["effects"];
				for (rapidjson::SizeType j = 0; j < effects.Size(); ++j)
				{
					auto& effect = effects[j];
					string effectName = effect.GetString();
					this->effectList[name].push_back(effectName);
					PipelineEffect* pe = EffectFactory::GetInstance().CreateEffect(effectName, lightSize);
					if(nullptr != pe)
						effectMask |= pe->mask;
				}
			}

			if (obj.HasMember("trans"))
				trans = Vector3{obj["trans"].GetString()};
			if(obj.HasMember("scale"))
				scale = Vector3{obj["scale"].GetString()};
			if (obj.HasMember("orien")) {
				auto& orien = obj["orien"];
				for (auto last=orien.Size(), i=last-last; i < last; i++) {
					auto axis = Vector3{orien[i]["axis"].GetString()};
					auto radian = orien[i]["rotate"].GetFloat();
					rotations.emplace_back(axis, radian);
				}
			}

			/*if (obj.HasMember("caster"))
			{
				const rapidjson::Value& casters = obj["caster"];
				for (rapidjson::SizeType j = 0; j < casters.Size(); ++j)
				{
					auto& caster = casters[j];
					string lightName = caster.GetString();
					this->caster[lightName].push_back(name);
				}
			}*/

			if (obj.HasMember("receiver"))
			{
				const rapidjson::Value& receivers = obj["receiver"];

				for (rapidjson::SizeType j = 0; j < receivers.Size(); ++j)
				{
					auto& receiver = receivers[j];
					unsigned int recvIdx = receiver.GetUint();
					effectMask |= (0x00000001u << (recvIdx - 1));
				}
			}

			auto meshMgr = MeshManager::getSingletonPtr();
			MeshPtr mesh = meshMgr->loadMesh_assimp_check(name, path);
			if (mesh == NULL) return;
			Entity* entity = this->getEntity(name);
			if (entity == NULL)
				entity = this->CreateEntity(name);
			entity->setMesh(mesh);
			SceneNode* snode = this->getSceneNode(name);
			if (snode == NULL) {
				snode = this->CreateSceneNode(name);
				snode->attachMovable(entity);
			}

			snode->AssignMask(effectMask);
			//snode->AssignMask(2u);

			snode->setTranslation(trans);
			snode->setScale(scale);
			for (auto& rot: rotations) {
				snode->rotate(rot.axis, rot.radian);
			}

			objectNode->attachNode(snode);
		}

		// --- test output ---
		/*ofstream ofs("save_output.json");
		OStreamWrapper osw(ofs);
		Writer<OStreamWrapper> writer(osw);
		d.Accept(writer);*/
	}

	SceneNode * SceneManager::CreateSceneNodeImp(const string & name)
	{
		//	printf("Create node %s",name.c_str());
		auto node = new SceneNode(name, this);
		m_sceneNodeRefList[name] = node;
		return node;
	}

	Light* SceneManager::CreateLight(const string &name, Light::LightType type)
	{
		Light *light = new Light(name, type, this);
		// handle a reference by manager
		if (m_lightRefList.find(light->getName()) != m_lightRefList.end())
		{

			printf("light %s already exists", name.c_str());
			assert(false && "light already exists.");
		}
		m_lightRefList.insert(std::make_pair(light->getName(), light));
		m_shadowRefList[light->getName()] = NULL;
		return light;

	}

	Entity* SceneManager::CreateEntity(const string &name)
	{
		Entity* ent = new Entity(name, this);
		if (m_entityRefList.find(name) != m_entityRefList.end())
		{

			printf("entity %s already exists", name.c_str());
			assert(false && "entity already exists.");
		}
		m_entityRefList.insert(std::make_pair(name, ent));
		ent->setVisible(true); // make this entity visible
		return ent;

	}

	Entity *SceneManager::CreateDynamicWeather(const string & /*name*/)
	{
		//ParticleSystem* ent = new DynamicWeather(name, this);
		//if (m_entityRefList.find("ParticleSystem") != m_entityRefList.end())
		//{

		//	printf("entity %s already exists", name.c_str());
		//	assert(false && "entity already exists.");
		//}
		//m_entityRefList.insert(std::make_pair(name, ent));
		//ent->setVisible(true); // make this entity visible
		//return ent;

		return NULL;
	}

	Camera* SceneManager::CreateCamera(const string &name)
	{
		Camera* camera = new Camera(name, this);
		assert(m_cameraRefList.find(name) == m_cameraRefList.end() && "camera already exists.");
		m_cameraRefList[name]= camera;

		return camera;
	}

	void SceneManager::addCamera(Camera* cam)
	{
		auto name = cam->getName();
		assert(m_cameraRefList.find(name) == m_cameraRefList.end() && "camera already exists.");
		m_cameraRefList[name] = cam;
	}

	// may have problem when the scenenode is a octreenode
	void SceneManager::destroy(SceneNode* node)
	{

		assert(node);
		std::map<string, SceneNode*>::iterator it = m_sceneNodeRefList.find(node->getName());
		if (it != m_sceneNodeRefList.end())
		{
			delete it->second;
			m_sceneNodeRefList.erase(it);
		}

	}

	void SceneManager::destroy(Entity* entity)
	{
		assert(entity);
		std::map<string, Entity*>::iterator it = m_entityRefList.find(entity->getName());
		if (it != m_entityRefList.end())
		{
			delete it->second;
			m_entityRefList.erase(it);
		}

	}

	void SceneManager::destroy(Light* light)
	{
		assert(light);
		std::map<string, Light*>::iterator it = m_lightRefList.find(light->getName());
		if (it != m_lightRefList.end())
		{
			delete it->second;
			m_lightRefList.erase(it);
		}
	}

	void SceneManager::destroy(Camera* camera)
	{
		assert(camera);
		std::map<string, Camera*>::iterator it = m_cameraRefList.find(camera->getName());
		if (it != m_cameraRefList.end())
		{
			delete it->second;
			m_cameraRefList.erase(it);
		}
	}

	Entity* SceneManager::getEntity(const string& name)
	{

		std::map<string, Entity*>::iterator it = m_entityRefList.find(name);
		if (it != m_entityRefList.end())
		{
			return it->second;
		}
		else
		{
			return NULL;
		}
	}

	Camera* SceneManager::getCamera(const string& name)
	{
		// try to convenient to get the default camera
		if (name.empty())
		{
			if (m_cameraRefList.size() != 0)
				return m_cameraRefList.begin()->second;
		}

		std::map<string, Camera*>::iterator it = m_cameraRefList.find(name);
		if (it != m_cameraRefList.end())
		{
			return it->second;
		}
		else
		{
			return NULL;
		}
	}

	Light * SceneManager::getLight(const string &name)
	{
		if (name.empty())
		{
			if (m_lightRefList.size() != 0)
				return m_lightRefList.begin()->second;
		}
		std::map<string, Light*>::iterator it = m_lightRefList.find(name);
		if (it != m_lightRefList.end())
		{
			return it->second;
		}
		else
		{
			return NULL;
		}
	}

	SceneNode* SceneManager::getSceneNode(const string& name) const
	{

		std::map<string, SceneNode*>::const_iterator it = m_sceneNodeRefList.find(name);
		if (it != m_sceneNodeRefList.end())
		{
			return it->second;
		}
		else
		{
			return NULL;
		}
	}

	void SceneManager::addChangedNode(SceneNode * node)
	{
		if (m_SceneRoot != node->getRoot())
			return;
		if (!influence_camera.empty())
		{
			influence_camera.clear();
			change_entity.clear();
		}

		const std::list<Entity *>& entity_list = node->getEntities();
		for (std::list<Entity *>::const_iterator move_it = entity_list.begin(); move_it != entity_list.end(); move_it++)
		{
			//std::map<String,Entity *>::iterator it = change_entity.find((*move_it)->getName());
			//if(change_entity.end() != it)
			//	return;

			// use the operator of [] to add or replace the entity;
			Entity * entity = dynamic_cast<Entity *>(*move_it);
			change_entity[entity->getName()] = entity;
		}

		if (node->hasChildNodes())
		{
			mapIterator<string, SceneNode *> children_begin = node->getChildBegin();
			mapIterator<string, SceneNode *> children_end = node->getChildEnd();
			for (; children_begin != children_end; children_begin++)
			{
				this->addChangedNode(children_begin.get());
			}
		}

	}

	void SceneManager::addInfluenceCamera(Camera * cam)
	{
		influence_camera[cam->getName()] = cam;
	}

	bool SceneManager::doneInfluence(Camera * came)
	{
		if (came)
		{
			return influence_camera.find(came->getName()) != influence_camera.end();
		}
		else
			return false;
	}

	Sphere SceneManager::getSceneBoundingSphere()
	{
		Sphere s;
		s.radius = 0;
		bool first = true;
		std::map<string, SceneNode*>::const_iterator it_scenenode = m_sceneNodeRefList.begin();
		for (; it_scenenode != m_sceneNodeRefList.end(); it_scenenode++)
		{
			if (m_SceneRoot == it_scenenode->second->getRoot())
			{
				if (first)
				{
					s = it_scenenode->second->getBoundingSphere();
					first = false;
				}
				else
				{
					SceneNode * sn = it_scenenode->second;
					if (sn->getEntities().empty() == false || sn->hasChildNodes())
						s.merge(it_scenenode->second->getBoundingSphere());
				}

			}

		}
		return s;
	}



	BoundingBox SceneManager::getSceneBoundingBox()
	{
		BoundingBox s;
		bool first = true;
		std::map<string, SceneNode*>::const_iterator it_scenenode = m_sceneNodeRefList.begin();
		for (; it_scenenode != m_sceneNodeRefList.end(); it_scenenode++)
		{
			if (m_SceneRoot == it_scenenode->second->getRoot())
			{
				if (first)
				{
					s = it_scenenode->second->getBoundingBox();
					first = false;
				}
				else
				{
					SceneNode * sn = it_scenenode->second;
					if (sn->getEntities().empty() == false || sn->hasChildNodes())
						s.merge(it_scenenode->second->getBoundingBox());
				}

			}

		}
		return s;
	}

	void SceneManager::setGlobalEffect(Effect * effect)
	{
		m_GlobalEffect = effect;
	}

	Effect * SceneManager::getGlobalEffect()
	{
		return m_GlobalEffect;
	}

	void SceneManager::setDepthEffect(Effect * effect)
	{
		m_depthEffect = effect;
	}

	Effect * SceneManager::getDepthEffect()
	{
		return m_depthEffect;
	}

	SceneManager::~SceneManager()
	{
		printf("sceneManager release");


		for (std::map<string, SceneNode*>::iterator it = m_sceneNodeRefList.begin(); it != m_sceneNodeRefList.end(); it++)
		{
			delete it->second;
		}
		m_sceneNodeRefList.clear();
		for (std::map<string, Light*>::iterator it = m_lightRefList.begin(); it != m_lightRefList.end(); it++)
		{
			delete it->second;
		}

		std::map<string, RenderTarget *>::iterator itr_rt = m_shadowRefList.begin();
		for (; itr_rt != m_shadowRefList.end(); itr_rt++)
		{
			if (itr_rt->second)
				delete itr_rt->second;;
		}
		m_lightRefList.clear();
		for (std::map<string, Camera*>::iterator it = m_cameraRefList.begin(); it != m_cameraRefList.end(); it++)
		{
			delete it->second;
		}
		m_cameraRefList.clear();

		for (std::map<string, Entity*>::iterator itr_entity = m_entityRefList.begin(); itr_entity != m_entityRefList.end(); itr_entity++)
		{
			delete itr_entity->second;
		}
		m_entityRefList.clear();
		//if (m_animation)
		//	delete m_animation;
		if (m_skyNode)
		{
			delete m_skyNode;
		}
		if (m_SceneRoot)
			delete m_SceneRoot;




	}

	SceneNode * SceneManager::createSkyNode(string mname /* = "skysphere" */)
	{
		m_skyNode = new SceneNode(mname, this);
		return m_skyNode;

	}

	Entity * SceneManager::getSkyEntity()
	{
		// skynode only has one entity;
		return dynamic_cast<Entity *>(*(m_skyNode->getEntities().begin()));
	}

	void SceneManager::findVisibleNode(Camera * cam, SceneNode * node)
	{
		/// we just ignore the null pointer
		/// and do not report an error.
		if (!node) return;
		/// test if this node has entity.
		if (node->hasEntities())
		{
			/// test bounding sphere
			//	if(cam->intersects(node->getBoundingSphere()))
			//	{
			/// continue test bounding box
			if (cam->intersects(node->getBoundingBox()))
			{

				const std::list<Entity*>& entities = node->getEntities();
				for (std::list<Entity*>::const_iterator it = entities.begin(); it != entities.end(); it++)
				{
					// test if this entity is visible
					//	cam->insertMoveableToRenderqueue(*it);
					cam->insertMoveableToRenderqueue(*it);

				}
			}

			intersect++;
			//	}
		}

		/// child node process
		if (node->hasChildNodes())
		{
			SceneNode::NodeIterator it = node->getChildBegin();
			SceneNode::NodeIterator end = node->getChildEnd();
			for (; it != end; it++)
			{
				findVisibleNode(cam, it.get());
			}
		}
	}

	void SceneManager::findVisibleNode(Camera *cam, SceneNode *node, RenderQueue &render_queue)
	{
		if (!node)
			return;
		if (node->hasEntities())
		{
			int cull = cam->intersects(node->getBoundingBox());
			if (cull == 0)
				return;
			const std::list<Entity*>& entities = node->getEntities();
			for (std::list<Entity*>::const_iterator it = entities.begin(); it != entities.end(); it++)
			{
				(*it)->updateRenderqueue(cam, render_queue);

			}
		}

		if (node->hasChildNodes())
		{
			SceneNode::NodeIterator it = node->getChildBegin();
			SceneNode::NodeIterator end = node->getChildEnd();
			for (; it != end; it++)
			{
				findVisibleNode(cam, it.get(), render_queue);
			}
		}
	}

	void SceneManager::getVisibleRenderQueue(Camera * cam)
	{
		//cam->v_render_queue.clear();
		if (true)
		{
			//	std::cout<<cam->getName()<<std::endl;
			findVisibleNode(cam, m_SceneRoot);

		}
		else
		{
			if (!change_entity.empty() && !this->doneInfluence(cam))
			{
				std::map<string, Entity*>::iterator entity_it = change_entity.begin();
				for (; entity_it != change_entity.end(); entity_it++)
				{
					cam->insertMoveableToRenderqueue(entity_it->second);
				}


			}
		}
		addInfluenceCamera(cam);
		cam->establishRenderQueue(true);
		//	std::cout<<cam->getName()<<" intersects "<<intersect<<" entity num = "<<m_entityRefList.size()<<std::endl;
		intersect = 0;
	}

	void SceneManager::getVisibleRenderQueue(Camera * cam, RenderQueue & render_queue)
	{
		findVisibleNode(cam, m_SceneRoot, render_queue);
	}



	void SceneManager::LoadScene(const string & /*scenefile_name*/, SceneNode* /*pRoot*/, bool /*castShadow*/)
	{
		//TseParser sceneParser(this, scenefile_name, pRoot, castShadow);
	}

	Entity * SceneManager::getScreeAlignedQuad()
	{
		if (!m_ScreenAlignedQuad)
			m_ScreenAlignedQuad = this->CreateEntity("ScreenAlignedQuad");
		return m_ScreenAlignedQuad;
	}

	void SceneManager::printStatistic()
	{
		printf("Node size = %d ,Entity size = %d", (int)m_sceneNodeRefList.size(), (int)m_entityRefList.size());
		for (std::map<string, SceneNode*>::iterator itr_sceneNode = m_sceneNodeRefList.begin(); itr_sceneNode != m_sceneNodeRefList.end(); itr_sceneNode++)
		{
			printf("sceneNode = %s", itr_sceneNode->second->getName().c_str());
		}
	}

	void SceneManager::releaseInternalRes()
	{
		std::map<string, RenderTarget *>::iterator itr_rt = m_shadowRefList.begin();
		for (; itr_rt != m_shadowRefList.end(); itr_rt++)
		{
			if (itr_rt->second)
				itr_rt->second->releaseInternalRes();
		}

		itr_rt = m_rendertargetList.begin();
		for (; itr_rt != m_rendertargetList.end(); itr_rt++)
		{
			if (itr_rt->second)
				itr_rt->second->releaseInternalRes();
		}
	}

	Sky * SceneManager::getSky()
	{
		return m_sky;

	}

	Sky* SceneManager::CreateSky(SkyTech /*tech*/, float /*radius_scale*/)
	{
		//switch (tech)
		//{
		//case ST_SkyScatter:
		//	m_sky = new SkyScattering(this, radius_scale);
		//	break;
		//case ST_SkyBox:
		//	m_sky = new SkyBox(this);
		//	break;
		//default:
		//	break;
		//}
		return m_sky;

	}


	HW::Sphere SceneManager::getSceneCastShadowSphere()
	{

		Sphere s;
		s.radius = 0;
		bool first = true;
		std::map<string, SceneNode*>::const_iterator it_scenenode = m_sceneNodeRefList.begin();
		for (; it_scenenode != m_sceneNodeRefList.end(); it_scenenode++)
		{
			if (m_SceneRoot == it_scenenode->second->getRoot())
			{
				SceneNode * sn = it_scenenode->second;

				//if (!sn->isCastShadow()) continue; //
				const std::list<Entity*> & entities = sn->getEntities();
				for (std::list<Entity*>::const_iterator itr_ent = entities.begin(); itr_ent != entities.end(); itr_ent++)
				{
					if ((*itr_ent)->visible() && (*itr_ent)->isCastShadow())
					{
						if (first)
						{
							s = (*itr_ent)->getBoundingSphere();
							first = false;
						}
						else
							s.merge((*itr_ent)->getBoundingSphere());
					}
				}


			}

		}
		return s;
	}

	void SceneManager::setActiveLightVector(Light * light, int index)
	{
		if (light) {
			assert(index < maximum_light_num);
			if (index >= (int)m_active_light_list.size())
				m_active_light_list.resize(index + 1, nullptr);
			m_active_light_list[index] = light;
		} else {
			assert(index <	(int)m_active_light_list.size());
			m_active_light_list.erase(m_active_light_list.begin() + index);
		}
	}

	const vector<Light *> & SceneManager::getActiveLightVetcor() const
	{
		return m_active_light_list;
	}

	const vector<Light *> & SceneManager::getShadowLightVetcor() const
	{

		////m_has_shadow_light_list.clear();
		//for (auto& l : m_active_light_list){
		//	if (l->isCastShadow()){
		//		m_has_shadow_light_list.push_back(l);
		//	}
		//}
		return m_has_shadow_light_list;
	}

	void SceneManager::updateLightStatus()
	{
		//auto e_shadow = dynamic_cast<ShadowMapGenerateEffect*>(EffectFactory::GetInstance().GetEffect("ShadowMapGenerateEffect"));
		//int i = 0;
		//for (auto light: m_has_shadow_light_list) {
		//	light->updateDirection();
		//	e_shadow->SetLightCamera(i++, light);
		//}
	}

	void SceneManager::updateShadowParam(Pass* pass) const
	{
		//auto shadowEffect = dynamic_cast<ShadowMapGenerateEffect*>(EffectFactory::GetInstance().GetEffect("ShadowMapGenerateEffect"));
		//auto p = pass;

		//// --- same ---
		//p->setProgramConstantData("lightNum", &shadowEffect->LightNum, "int", sizeof(int));
		//// --- diff ---
		//for (int i = 0; i < shadowEffect->LightNum; ++i)
		//{
		//	auto light = m_has_shadow_light_list[i];
		//	p->setProgramConstantData("shadowMap[" + to_string(i) + "]", shadowEffect->out_ShadowMapTexture[i]);
		//	p->setProgramConstantData("ShadowMatrix[" + to_string(i) + "]", shadowEffect->out_ShadowMatrix[i]->ptr(), "mat4", sizeof(Matrix4));
		//	Matrix4 lightview = shadowEffect->cameras[i].getViewMatrix();
		//	p->setProgramConstantData("g_lightView[" + to_string(i) + "]", lightview.ptr(), "mat4", sizeof(Matrix4));
		//	p->setProgramConstantData("LightColor[" + to_string(i) + "]", light->Diffuse.ptr(), "vec3", sizeof(Vector3));
		//	p->setProgramConstantData("LightPosition[" + to_string(i) + "]", light->getPosition().ptr(), "vec3", sizeof(Vector3));
		//	p->setProgramConstantData("g_shadowTechnique[" + to_string(i) + "]", &light->tech, "int", sizeof(int));
		//	Vector2 lr(light->radius / shadowEffect->cameras[i].getRight() * 2, light->radius / shadowEffect->cameras[i].getTop() * 2);
		//	p->setProgramConstantData("g_lightRadiusUV[" + to_string(i) + "]", lr.ptr(), "vec2", sizeof(Vector2));
		//	p->setProgramConstantData("g_lightZNear", &light->near, "float", sizeof(float));
		//	p->setProgramConstantData("g_lightZFar", &light->far, "float", sizeof(float));
		//	p->setProgramConstantData("bias", &light->bias, "float", sizeof(float));
		//	p->setProgramConstantData("g_samplePattern", &light->samplePattern, "int", sizeof(int));
		//}
	}

	void SceneManager::getVisibleRenderQueue_as(Camera * /*cam*/, RenderQueue & renderQueue)
	{
		//暂时先获取全部mesh for test
		for (auto&x : m_SceneRoot->getChildNode("objectNode")->getAllChildNodes()) {
			auto entitys = x->getEntities();
			for (auto& entity : entitys) {
				for (auto& y : entity->getMesh()->m_SubMeshList_as) {
					auto mesh = y.second;

					RenderQueueItem item;
					item.entity = entity;
					item.asMesh = mesh;
					renderQueue.push_back(item);
				}
			}
		}
	}

	void SceneManager::pushNodeToQueue(string name, RenderQueue & renderQueue) const
	{
		if ("quad" == name)
		{
			RenderUtil::GetScreenQuad(renderQueue);
			return;
		}

		SceneNode* node = this->getSceneNode(name);
		auto entities = node->getEntities();
		for (auto& e : entities)
		{
			for (auto& m : e->getMesh()->m_SubMeshList_as)
			{
				auto mesh = m.second;
				RenderQueueItem item;
				item.entity = e;
				item.asMesh = mesh;
				renderQueue.push_back(item);
			}
	}
	}


}

SceneContainer::SceneContainer()
{

}

