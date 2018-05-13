#include "GlobePipeline.h"
#include "../RenderTarget.h"
#include "../SceneManager.h"
#include "../mathlib.h"
#include "../Camera.h"
#include "../CameraControl.h"
///////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////
void WeatherEffect::Init()
{
    //where to render(main screen)
    auto& rt_mgr = RenderTargetManager::getInstance();
    auto mainwindow = rt_mgr.get("MainWindow");

    //add shader
    weather_pass = PassManager::getInstance().LoadPass("weather_pass", "RayCastedGlobe/weather_prog.json");
    weather_pass->renderTarget = mainwindow;
    weather_pass->mClearState.clearFlag = false;

    //fbcMgr->Init();

}

void WeatherEffect::Update()
{
    auto scene = SceneContainer::getInstance().get(in_scenename);
    auto camera = scene->getCamera(in_cameraname);

    RenderQueue queue;
    scene->getVisibleRenderQueue_as(camera, queue);

    auto mat4fToMatrix4 = [](mat4f m) {
        Matrix4 matrix;//transpose
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                matrix[i][j] = m[j][i];
            }
        }
        return matrix;
    };

    //set light information
    auto light_color = Vector3{ 1.0f, 1.0f, 1.0f };
    auto light_dir = Vector3{ 1.0f, 0.0f, 0.0f };
    weather_pass->setProgramConstantData("lightDirection", light_dir.ptr(), "vec3", sizeof(Vector3));
    weather_pass->setProgramConstantData("lightColor", light_color.ptr(), "vec3", sizeof(Vector3));

    //set camera parameters
    Matrix4 ScaleMatrix= Matrix4::IDENTITY;
    ScaleMatrix.setScale(Vector3(1000000, 1000000, 1000000));

    Matrix4 TransMatrix = Matrix4::IDENTITY;
    if (globeInteractive->target_longtitude == 0)
    {
        TransMatrix.setTrans(Vector3(main_camera->position().x, main_camera->position().y, main_camera->position().z));
    }
    else
    {
        Geodetic3D local_pos(globeInteractive->target_longtitude, globeInteractive->target_latitude, earthRadius/10);
        vec3d world_pos = earthshape->ToVector3D(local_pos);
        TransMatrix.setTrans(Vector3(world_pos.x, world_pos.y, world_pos.z));
    }
    Matrix4 worldMatrix = TransMatrix * ScaleMatrix;
    Matrix4 worldToCamera = mat4fToMatrix4(main_camera->m_view_matrix());
    Matrix4 cameraToScreen = mat4fToMatrix4(main_camera->m_absolute_projection_matrix());
    Vector3 cameraEye = Vector3(main_camera->position().x, main_camera->position().y, main_camera->position().z);
    weather_pass->setProgramConstantData("eyePosition", cameraEye.ptr(), "vec3", sizeof(Vector3));
    Matrix4 localToScreen = cameraToScreen * worldToCamera;
    weather_pass->setProgramConstantData("ModelMatrix", worldMatrix.ptr(), "mat4", sizeof(Matrix4));
    weather_pass->setProgramConstantData("ViewPerspectiveMatrix", localToScreen.ptr(), "mat4", sizeof(Matrix4));

    weather_pass->camera = camera;
    weather_pass->queue = queue;
}

void WeatherEffect::GetInputPasses(vector<Pass*>& passes, vector<pair<Pass*, Pass*>>& /*inputPasses*/, vector<pair<Texture*&, Texture*&>>& inputTexture)
{
    passes.push_back(weather_pass);
}

///////////////////////////////////////////////////////////////////////////

void RayCastedGlobeEffect::Init()
{
    auto& rt_mgr = RenderTargetManager::getInstance();
    auto mainwindow = rt_mgr.get("MainWindow");
    RayCastedGlobe_pass = PassManager::getInstance().LoadPass("RayCastedGlobe_pass", "RayCastedGlobe/RayCastedGlobe_prog.json");
    RayCastedGlobe_pass->renderTarget = mainwindow;
}

void RayCastedGlobeEffect::Update()
{

    //original uniforms
    auto scene = SceneContainer::getInstance().get(in_scenename);
    auto camera = scene->getCamera(in_cameraname);

    auto mat4fToMatrix4 = [](mat4f m){
        Matrix4 matrix;//transpose
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                matrix[i][j] = m[j][i];
            }
        }
        return matrix;
    };

    //set light information
    Vector3 lightDir = Vector3(1, 1, 0);
    RayCastedGlobe_pass->setProgramConstantData("u_lightDir", lightDir.ptr(), "vec3", sizeof(Vector3));

    //set globe information
    double radiiSquared = 1.0 / (earthRadius* earthRadius);
    Vector3 globeOneOverRadiiSquared = Vector3(radiiSquared, radiiSquared, radiiSquared);
	Vector3 atmosOneOverRadiiSquared = globeOneOverRadiiSquared / 1.01 / 1.01;
    RayCastedGlobe_pass->setProgramConstantData("u_globeOneOverRadiiSquared", globeOneOverRadiiSquared.ptr(), "vec3", sizeof(Vector3));
	RayCastedGlobe_pass->setProgramConstantData("u_atmosOneOverRadiiSquared", atmosOneOverRadiiSquared.ptr(), "vec3", sizeof(Vector3));


    //set camera parameters
    Matrix4 worldToCamera = mat4fToMatrix4(main_camera->m_view_matrix());
    Matrix4 cameraToScreen = mat4fToMatrix4(main_camera->m_absolute_projection_matrix());
    Vector3 renf = Vector3(main_camera->reference_center().x, main_camera->reference_center().y, main_camera->reference_center().z);
    RayCastedGlobe_pass->setProgramConstantData("u_rte", renf.ptr(), "vec3", sizeof(Vector3));
    Vector3 cameraEye = Vector3(main_camera->position().x, main_camera->position().y, main_camera->position().z);
    Vector3 cameraEyeSquared = Vector3(cameraEye.x*cameraEye.x, cameraEye.y*cameraEye.y, cameraEye.z*cameraEye.z);
    RayCastedGlobe_pass->setProgramConstantData("og_cameraEye", cameraEye.ptr(), "vec3", sizeof(Vector3));
    RayCastedGlobe_pass->setProgramConstantData("u_cameraEyeSquared", cameraEyeSquared.ptr(), "vec3", sizeof(Vector3));
    Matrix4 localToScreen = cameraToScreen * worldToCamera;
    RayCastedGlobe_pass->setProgramConstantData("og_modelViewPerspectiveMatrix", localToScreen.ptr(), "mat4", sizeof(Matrix4));
    /*Vector3 cameraEye = Vector3(main_camera->position().x, main_camera->position().y, main_camera->position().z);
    Vector3 cameraEyeSquared = Vector3(cameraEye.x*cameraEye.x, cameraEye.y*cameraEye.y, cameraEye.z*cameraEye.z);
    Vector3 camera_direction = Vector3(main_camera->getDir().x, main_camera->getDir().y, main_camera->getDir().z);
    Vector3 camera_up = Vector3(main_camera->getCameraUpVector().x, main_camera->getCameraUpVector().y, main_camera->getCameraUpVector().z);
    float fov = main_camera->fovx();
    float aspect = main_camera->aspect();
    RayCastedGlobe_pass->setProgramConstantData("og_cameraEye", cameraEye.ptr(), "vec3", sizeof(Vector3));
    RayCastedGlobe_pass->setProgramConstantData("u_cameraEyeSquared", cameraEyeSquared.ptr(), "vec3", sizeof(Vector3));
	RayCastedGlobe_pass->setProgramConstantData("camera_direction", camera_direction.ptr(), "vec3", sizeof(Vector3));
	RayCastedGlobe_pass->setProgramConstantData("camera_up", camera_up.ptr(), "vec3", sizeof(Vector3));
	RayCastedGlobe_pass->setProgramConstantData("fov", &fov, "float", sizeof(float));
	RayCastedGlobe_pass->setProgramConstantData("aspect", &aspect, "float", sizeof(float));*/

    //gain all ready-to-render object
    RenderQueue queue;
    scene->getVisibleRenderQueue_as(camera, queue);

    RayCastedGlobe_pass->camera = camera;
    RayCastedGlobe_pass->queue = queue;
}

void RayCastedGlobeEffect::GetInputPasses(vector<Pass*>& passes, vector<pair<Pass*, Pass*>>& /*inputPasses*/, vector<pair<Texture*&, Texture*&>>& inputTexture)
{
    passes.push_back(RayCastedGlobe_pass);
}

///////////////////////////////////////////////////////////////////////////
void fbcEffect::Init()
{
    auto& rt_mgr = RenderTargetManager::getInstance();
    auto mainwindow = rt_mgr.get("MainWindow");
    fbc_pass = PassManager::getInstance().LoadPass("fbc_pass", "fengbaochao/fengbaochao.json");
    fbc_pass->renderTarget = mainwindow;
    fbcManager_->fbc_->fbc_pass = fbc_pass;
}

void fbcEffect::Update()
{
    fbcManager_->fbc_->update();
    auto scene = SceneContainer::getInstance().get(in_scenename);
    auto camera = scene->getCamera(in_cameraname);

    RenderQueue queue;
    scene->getVisibleRenderQueue_as(camera, queue);
    fbc_pass->camera = camera;
    fbc_pass->queue = queue;
}

void fbcEffect::GetInputPasses(vector<Pass*>& passes, vector<pair<Pass*, Pass*>>& /*inputPasses*/, vector<pair<Texture*&, Texture*&>>& inputTexture)
{
    passes.push_back(fbc_pass);
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
    else //this way
    {
        //draw the earth
        fx_main_raycasted = new RayCastedGlobeEffect(main_camera, earthshape, earthRadius, globeInteractive);
        fx_main_raycasted->in_scenename = "scene1";
        fx_main_raycasted->in_cameraname = "main";
        fx_main_raycasted->Init();
        //weather effect
        weather_effect = new WeatherEffect(main_camera, earthshape, earthRadius, globeInteractive);
        weather_effect->in_scenename = "scene_weather";
        weather_effect->in_cameraname = "main";
        weather_effect->Init();
        //fengbaochao effect
        fbc_effect = new fbcEffect(main_camera, earthshape, fbcManager_);
        fbc_effect->in_scenename = "scene_fengbaochao";
        fbc_effect->in_cameraname = "main";
        fbc_effect->Init();
        passGraph.AttachEffect(fx_main_raycasted);
        //passGraph.AttachEffect(weather_effect);
        passGraph.AttachEffect(fbc_effect);
        passGraph.PrintGraph();
    }
}

void GlobePipeline::Render()
{
    ////////////////////////////////////////////////
    globeInteractive->perFrameInteractive();
    main_camera->update();
    if (globeInteractive->placeObj == true && hasAttachedWeather == false)
    {
        std::cout << "attached weather effects" << std::endl;
        passGraph.AttachEffect(weather_effect);
        hasAttachedWeather = true;
    }
    if (globeInteractive->placeObj == false && hasAttachedWeather == true)
    {
        std::cout << "deattached weather effects" << std::endl;
        passGraph.DetachEffect(weather_effect);
        hasAttachedWeather = false;
    }
    /////////////////////////////////////////////////
    passGraph.Update();
    passGraph.Render();
}
