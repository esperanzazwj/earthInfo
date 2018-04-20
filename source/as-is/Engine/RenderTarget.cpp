#include "RenderTarget.h"
#include "RenderSystem.h"
#include "Camera.h"
#include "EngineUtil.h"
#include "TextureManager.h"
using namespace rapidjson;
namespace HW
{
	RenderTarget::RenderTarget(const string &_name):RenderTarget()
	{
		this->name = _name;
	}

	RenderTarget::RenderTarget()
	{
		m_valid = false;
		auto system = GlobalResourceManager::getInstance().m_RenderSystem;
		system->GetWandH(width, height);
	}

	void RenderTarget::setDimension(unsigned int width, unsigned int height)
	{
		this->width = width;
		this->height = height;
	}

	unsigned int RenderTarget::getWidth()
	{
		return width;
	}

	unsigned int RenderTarget::getHeight()
	{
		return height;
	}

	void RenderTarget::resize(int w, int h)
	{
		this->width = w;
		this->height = h;
		for (auto& texpair : m_attach_textures) {
			auto tex = texpair.second;
			tex->ReSize(w, h);
		}
		RebindAttachTextures_Internal();
	}

}

void RenderTargetManager::LoadRenderTargetPreset(string filename)
{
	string file_content = file_util::slurp(filename);

	m_presets_doc.Parse(file_content.c_str());

	auto& ps = m_presets_doc["RenderTarget_presets"];
	rapidjson::Value v;
	for (auto it = ps.MemberBegin(); it != ps.MemberEnd(); it++){
		v = it->value;
		string name = v["preset_name"].GetString();
		presets[name] = v;
	}
}

RenderTarget* RenderTargetManager::CreateRenderTargetFromPreset(string preset_name,string rt_name, int width , int height, int depth)
{
	auto it = presets.find(preset_name);
	if (it == presets.end())
		return NULL;
	
	RenderTarget* rt = GlobalResourceManager::getInstance().m_RenderTargetFactory->create();
	
	rt->name = rt_name;
	if (resources.find(rt_name) != resources.end())
		return resources[rt_name];

	Value& preset = it->second;

	//get framebuffer size
	if (width == 0 || height == 0) {
		if (preset["size"].IsString()) {
			string size = preset["size"].GetString();
			if (size == "MainWindow" || size == "") {
				auto mw = RenderTargetManager::getInstance().get("MainWindow");
				width = mw->getWidth();
				height = mw->getHeight();
			}
		}
		else {
			width = preset["size"]["width"].GetInt();
			height = preset["size"]["height"].GetInt();
		}
	}
	rt->width = width;
	rt->height = height;
	auto& tex_descs = preset["RenderTextures"];
	for (auto it = tex_descs.MemberBegin(); it != tex_descs.MemberEnd(); it++){
		auto& tm=TextureManager::getInstance();
		//create texture
		string tex_preset_name = it->value["preset"].GetString();
		Texture* tex = tm.CreateTextureFromPreset(tex_preset_name);

		//set texture name
		string texname = rt_name + "_" + it->name.GetString();
		tex->name = texname;

		//set size
		tex->width = width;
		tex->height = height;
		tex->depth = depth;

		//add to manager
		tm.add(tex);


		//get attachment
		string attach=it->value["attachment"].GetString();

		rt->m_attach_textures[attach] = tex;
	}
	add(rt);
	return rt;
}

RenderTargetManager::RenderTargetManager()
{

}
