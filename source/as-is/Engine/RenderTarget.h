#pragma once

#include<string>
#include "NameGenerator.h"
#include "PreDefine.h"
#include "ResourceManager.h"
#include <cereal/external/rapidjson/document.h>
#include <map>
#include <vector>
using namespace std;
namespace HW
{
	class RenderSystem;
	class Camera;
	
	class RenderTarget
	{
	public: 
		enum AttachComponent {
			COLOR0,
			COLOR1,
			COLOR2,
			COLOR3,
			COLOR4,
			COLOR5,
			COLOR6,
			COLOR7,
			COLOR8,
			DEPTH,
			AttachComponentCount
		};

		/// real functions
		RenderTarget();
		RenderTarget(const string &name);
		/// virtual functions
		virtual void createInternalRes() = 0;
		virtual void releaseInternalRes() = 0;
		virtual void bindTarget(int index = 0) = 0;

		virtual void attachTexture(Texture* texture, string attach_component){
			m_attach_textures[attach_component] = texture;
		}

		//make sure already createInternalResource
		virtual void attachTextureCubeMapNow(Texture* cubetexture, string attach_component, int /*face*/){
			attachTexture(cubetexture, attach_component);
		}
		


		virtual void setDimension(unsigned int width, unsigned int height);
		virtual unsigned int getWidth();
		virtual unsigned int getHeight();

		virtual void RebindAttachTextures_Internal(){}

		virtual void BlitTexture(RenderTarget* /*out*/,string /*SrcAttach*/, string /*DstAttach*/) {}

		virtual void resize(int w, int h);
		string name;
		map<string, Texture*> m_attach_textures;

		int width;
		int height;
		virtual ~RenderTarget() {
			//std::cout<<"RenderTaregt "<<m_Name<<" Destruct"<<std::endl;
		}
	protected:

		bool m_valid;
		
		
	};
}

class  RenderTargetFactory
{
public:
	virtual RenderTarget * create() = 0;

};

class RenderTargetManager :public ResourceManager<RenderTarget*>{
public:
	static RenderTargetManager& getInstance()
	{
		static RenderTargetManager    instance;
		return instance;
	}

	void LoadRenderTargetPreset(string filename);
	RenderTarget* CreateRenderTargetFromPreset(string preset_name, string rt_name,int width=0,int height=0, int depth = 0);


private:
	rapidjson::Document m_presets_doc;
	map<string, rapidjson::Value> presets;
	RenderTargetManager();
	RenderTargetManager(RenderTargetManager const&);              // Don't Implement.
	void operator=(RenderTargetManager const&); // Don't implement
	NameGenerator* namegen;
};