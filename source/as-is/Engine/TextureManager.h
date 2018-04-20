#pragma once
#include "../../resource/image.hpp"
#include "Texture.h"
#include "NameGenerator.h"
#include "ResourceManager.h"
#include <cereal/external/rapidjson/document.h>
#include <cereal/external/rapidjson/filereadstream.h>
using namespace rapidjson;
using namespace HW;
using namespace std;


class TextureManager: public ResourceManager<Texture*>{
public:
	using Image = ss::resource::Image;
	using Image_Layout = ss::resource::Image_Layout;

	static TextureManager& getInstance()
	{
		static TextureManager    instance;
		return instance;
	}

	Texture* CreateTextureFromImage(string path, string texname="", Image_Layout il=Image_Layout::u8x3);
	void LoadTexturePreset(string filename);
	Texture* CreateTextureFromPreset(string preset_name);
	Texture* CreateTextureCubeMap(vector<string>& paths, string texname = "", Image_Layout il=Image_Layout::f32x4);
	Texture* CreateTextureCubeMapEmpty(string texname);

private:
	rapidjson::Document m_presets_doc;
	map<string, rapidjson::Value> presets;
	TextureManager();
	TextureManager(TextureManager const&);              // Don't Implement.
	void operator=(TextureManager const&); // Don't implement
	NameGenerator* namegen;
};

