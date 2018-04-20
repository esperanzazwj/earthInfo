#include "GlobePipeline.h"
#include "../RenderTarget.h"
#include "../SceneManager.h"
#include "../mathlib.h"
#include "../Camera.h"
#include "../CameraControl.h"

void GlobeEffect::Init()
{
	auto& rt_mgr = RenderTargetManager::getInstance();
	auto mainwindow = rt_mgr.get("MainWindow");

	globe_pass = PassManager::getInstance().LoadPass("globe_pass", "globe/globe_prog.json");
	globe_pass->renderTarget = mainwindow;
}

void GlobeEffect::Update()
{
	auto scene = SceneContainer::getInstance().get(in_scenename);
	auto camera = scene->getCamera(in_cameraname);

	RenderQueue queue;
	scene->getVisibleRenderQueue_as(camera, queue);

	auto light_color = Vector3{1.0f, 1.0f, 1.0f};
	auto light_pos = Vector3{10.0f, 0.0f, 0.0f};
	globe_pass->setProgramConstantData("lightColor", light_color.ptr(), "vec3", sizeof(Vector3));
	globe_pass->setProgramConstantData("lightPosition", light_pos.ptr(), "vec3", sizeof(Vector3));
	globe_pass->camera = camera;
	globe_pass->queue = queue;
}

void GlobeEffect::GetInputPasses(vector<Pass*>& passes, vector<pair<Pass*, Pass*>>& /*inputPasses*/, vector<pair<Texture*&, Texture*&>>& inputTexture)
{
	passes.push_back(globe_pass);
}

void GlobePipeline::Init()
{
    //Switch Effect
    bool loadingOldGlobeEffect = false;
    if (loadingOldGlobeEffect)
    {
        fx_main = new GlobeEffect;
	    fx_main->in_scenename = "scene1";
	    fx_main->in_cameraname = "main";
	    fx_main->Init();
        passGraph.AttachEffect(fx_main);
        passGraph.PrintGraph();
    }
    else
    {
        fx_main_raycasted = new RayCastedGlobeEffect(main_camera, earthshape, earthRadius);
        fx_main_raycasted->in_scenename = "scene1";
        fx_main_raycasted->in_cameraname = "main";
        fx_main_raycasted->Init();
        passGraph.AttachEffect(fx_main_raycasted);
        passGraph.PrintGraph();
    }


}

void GlobePipeline::Render()
{
    main_camera->update();
	passGraph.Update();
	passGraph.Render();
}

void RayCastedGlobeEffect::Init()
{
    auto& rt_mgr = RenderTargetManager::getInstance();
    auto mainwindow = rt_mgr.get("MainWindow");
    RayCastedGlobe_pass = PassManager::getInstance().LoadPass("RayCastedGlobe_pass", "RayCastedGlobe/RayCastedGlobe_prog.json");
    RayCastedGlobe_pass->renderTarget = mainwindow;
}

void RayCastedGlobeEffect::Update()
{
    auto scene = SceneContainer::getInstance().get(in_scenename);
    auto camera = scene->getCamera(in_cameraname);
	
	vec3d cameraEye_main = main_camera->position();
	Vector3 cameraEye = Vector3(cameraEye_main.x, cameraEye_main.y, cameraEye_main.z);
	vec3d camera_direction_main = main_camera->getDir();
	Vector3 camera_direction = Vector3(camera_direction_main.x, camera_direction_main.y, camera_direction_main.z);
	vec3d camera_up_main = main_camera->getCameraUpVector();
	Vector3 camera_up = Vector3(camera_up_main.x, camera_up_main.y, camera_up_main.z);
	float fov = main_camera->fovx();
	float aspect = main_camera->aspect();

	globeInteractive->perFrameInteractive();

	//gain all ready-to-render object
    RenderQueue queue;
    scene->getVisibleRenderQueue_as(camera, queue);

    //RayCastedGlobe
    Vector3 lightDir = Vector3(1, 1, 0);
    RayCastedGlobe_pass->setProgramConstantData("u_lightDir", lightDir.ptr(), "vec3", sizeof(Vector3));

    double radiiSquared = 1.0 / (earthRadius* earthRadius);
    Vector3 globeOneOverRadiiSquared = Vector3(radiiSquared, radiiSquared, radiiSquared);
	Vector3 atmosOneOverRadiiSquared = globeOneOverRadiiSquared / 1.01 / 1.01;
    RayCastedGlobe_pass->setProgramConstantData("u_globeOneOverRadiiSquared", globeOneOverRadiiSquared.ptr(), "vec3", sizeof(Vector3));
	RayCastedGlobe_pass->setProgramConstantData("u_atmosOneOverRadiiSquared", atmosOneOverRadiiSquared.ptr(), "vec3", sizeof(Vector3));
	//Vector3 cameraEye = camera->getPosition();
	Vector3 cameraEyeSquared = Vector3(cameraEye.x*cameraEye.x, cameraEye.y*cameraEye.y, cameraEye.z*cameraEye.z);
    RayCastedGlobe_pass->setProgramConstantData("og_cameraEye", cameraEye.ptr(), "vec3", sizeof(Vector3));
    RayCastedGlobe_pass->setProgramConstantData("u_cameraEyeSquared", cameraEyeSquared.ptr(), "vec3", sizeof(Vector3));
    //camera uniform 
	//Vector3 camera_direction = camera->getDirection();
	//Vector3 camera_up = camera->getUp();
	//float fov = camera->getFov();
	//float aspect = camera->getAspect();
	RayCastedGlobe_pass->setProgramConstantData("camera_direction", camera_direction.ptr(), "vec3", sizeof(Vector3));
	RayCastedGlobe_pass->setProgramConstantData("camera_up", camera_up.ptr(), "vec3", sizeof(Vector3));
	RayCastedGlobe_pass->setProgramConstantData("fov", &fov, "float", sizeof(float));
	RayCastedGlobe_pass->setProgramConstantData("aspect", &aspect, "float", sizeof(float));
    RayCastedGlobe_pass->camera = camera;
    RayCastedGlobe_pass->queue = queue;
}

void RayCastedGlobeEffect::GetInputPasses(vector<Pass*>& passes, vector<pair<Pass*, Pass*>>& /*inputPasses*/, vector<pair<Texture*&, Texture*&>>& inputTexture)
{
    passes.push_back(RayCastedGlobe_pass);
}
