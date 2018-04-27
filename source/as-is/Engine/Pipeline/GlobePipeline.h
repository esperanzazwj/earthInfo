#pragma once
#include "../Pipeline.h"
#include "../PreDefine.h"
#include <vector>
#include "../../oceanInfo/Camera/worldCamera.h"
#include "../CameraControl.h"
//#include "../../app/Globe.h"
using namespace HW;

struct GlobeEffect: PipelineEffect
{
	GlobeEffect() { this->name = "globe"; }
	virtual void Init();
	virtual void Update();
	virtual void GetInputPasses(vector<Pass*>&, vector<pair<Pass*, Pass*>>&, vector<pair<Texture*&, Texture*&>>&);

	string in_scenename;
	string in_cameraname;

private:
	RenderTarget* rt_out{};
    Pass* globe_pass{};
};

struct WeatherEffect : PipelineEffect
{
	WeatherEffect(VirtualGlobeScene::MomentumCamera *main_camera, Ellipsoid *earthshape, float earthRadius, GlobeInteractive *globeInteractive):
		main_camera(main_camera), earthshape(earthshape), earthRadius(earthRadius), globeInteractive(globeInteractive)
	{ this->name = "weather"; }
	virtual void Init();
	virtual void Update();
	virtual void GetInputPasses(vector<Pass*>&, vector<pair<Pass*, Pass*>>&, vector<pair<Texture*&, Texture*&>>&);

	string in_scenename;
	string in_cameraname;

private:
	RenderTarget * rt_out{};
	Pass* weather_pass{};
	//Camera Controller
	GlobeInteractive * globeInteractive;
	VirtualGlobeScene::MomentumCamera *main_camera;
	Ellipsoid *earthshape;
	float earthRadius;
};

struct RayCastedGlobeEffect : PipelineEffect
{
public:
	RayCastedGlobeEffect(VirtualGlobeScene::MomentumCamera *main_camera, Ellipsoid *earthshape, float earthRadius, GlobeInteractive *globeInteractive)
		:main_camera(main_camera), earthshape(earthshape), earthRadius(earthRadius), globeInteractive(globeInteractive)
	{
		this->name = "RayCastedGlobe";
		//globeInteractive = new GlobeInteractive(main_camera, earthshape);// app::globeInteractive;
		assert(globeInteractive != NULL);
	}
	~RayCastedGlobeEffect()
	{
		if (globeInteractive != NULL)
			delete globeInteractive;
	}
	virtual void Init();
	virtual void Update();
	virtual void GetInputPasses(vector<Pass*>&, vector<pair<Pass*, Pass*>>&, vector<pair<Texture*&, Texture*&>>&);

	string in_scenename;
	string in_cameraname;
private:
	RenderTarget * rt_out{};
	Pass* RayCastedGlobe_pass{};
	//Camera Controller
	GlobeInteractive * globeInteractive;
	VirtualGlobeScene::MomentumCamera *main_camera;
	Ellipsoid *earthshape;
	float earthRadius;
};


struct GlobePipeline : Pipeline
{
public:
	GlobePipeline(VirtualGlobeScene::MomentumCamera *main_camera = NULL, Ellipsoid *earthshape = NULL, float earthRadius = 0, GlobeInteractive *globeInteractive = NULL)
		:main_camera(main_camera), earthshape(earthshape), earthRadius(earthRadius),
		globeInteractive(globeInteractive), fx_main(NULL), fx_main_raycasted(NULL), hasAttachedWeather(false)
	{}
	virtual void Init();
	virtual void Render();
private:
	GlobeEffect * fx_main;//obsolete
	RayCastedGlobeEffect * fx_main_raycasted;
	WeatherEffect *weather_effect;

	float earthRadius;
	VirtualGlobeScene::MomentumCamera *main_camera;
	Ellipsoid *earthshape;
	GlobeInteractive * globeInteractive;
	bool hasAttachedWeather;
};