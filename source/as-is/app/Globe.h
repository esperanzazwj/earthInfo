#pragma once
#include "../Engine/AppFrameWork2.h"
#include "../Engine/Pipeline/GlobePipeline.h"
#include "../../oceanInfo/Geometry/Ellipsoid.h"
#include "../../oceanInfo/Camera/worldCamera.h"
#include "../Engine/CameraControl.h"
#include "../../fengbaochao/fbcManager.h"

namespace app
{

    //Camera Controller
    GlobeInteractive * globeInteractive = NULL;

    void scrollMoveEvent(GLFWwindow *window, double xoffset, double yoffset)
    {
        cout << "offset: " << xoffset << "," << yoffset << endl;
        GlobeInteractive *globe_interactive = app::globeInteractive;
        double scale = 30.0;
        if (globe_interactive->camera->height_above_terrian() < 1000)
            scale = 50.0;
        globe_interactive->camera->zoomStepped(yoffset * globe_interactive->moving_scale * globe_interactive->move_step / scale);
    }

    //windows class
    struct Globe: App
    {
        float earthRadius;
        VirtualGlobeCore::Ellipsoid *earthshape; // 地球数学模型
        VirtualGlobeScene::MomentumCamera *main_camera; // 主相机
        RenderTarget * basic_buffer;

        SceneManager *scene_weather;
        SceneManager *scene_fengbaochao;

        fbcManager * fbcManager_;

        Globe() :earthRadius(0), earthshape(NULL), main_camera(NULL), basic_buffer(NULL) {}

        //entry
        void Init()
        {
            name = "globe";
            render = new EGLRenderSystem;
            render->GetWandH(w, h);     // Reset framebuffer size because it may be different than what you set.
            render->Initialize();
            //initialize earthshape
            earthRadius = 6378137.0;
            earthshape = new VirtualGlobeCore::Ellipsoid(earthRadius, earthRadius, earthRadius);//should release
            //main camera
            main_camera = new VirtualGlobeScene::MomentumCamera(earthshape);//should release
            main_camera->set_viewport(vec4i(0, 0, w, h));

            fbcManager_ = new fbcManager(earthshape, main_camera);
            InitWrapper();//call create_scene

            basic_buffer = RenderTargetManager::getInstance().CreateRenderTargetFromPreset("basic", "basic_buffer");
            basic_buffer->createInternalRes();

            //camera controller
            app::globeInteractive = new GlobeInteractive(w, h, main_camera, earthshape);


            auto& ctx = ss::Window_System::current().context();
            glfwSetScrollCallback(ctx, app::scrollMoveEvent);

            //initalize pipeline
            InitPipeline();
        }

        void InitPipeline()
        {
            //Globe pipeline(define how to shade)
            pipeline = new GlobePipeline(main_camera, earthshape, earthRadius, app::globeInteractive,fbcManager_);
            pipeline->Init();
        }

        //call this function per frame
        void Render()
        {
            UpdateGUI();
            pipeline->Render();
        }

        //initalized scene(model)
        void CreateScene()
        {
            //create scene manager(not yet load model)
            scene = new OctreeSceneManager("scene1", render);
            scene_weather= new OctreeSceneManager("scene_weather", render);
            scene_fengbaochao = new OctreeSceneManager("scene_fengbaochao", render);

            //scene manager-->SceneContainer(global)
            SceneContainer::getInstance().add(scene);
            SceneContainer::getInstance().add(scene_weather);
            SceneContainer::getInstance().add(scene_fengbaochao);

            //obsolete
			Vector3 cameraEye = Vector3(earthRadius * 3, 0, 0);
			Vector3 camera_direction = Vector3(-1, 0, 0);
			Vector3 camera_up = Vector3(0, 0, 1);
            camera = scene->CreateCamera("main");
			
           // camera->lookAt(Vector3(0, 0, 2.5), Vector3(0, 0, 0), Vector3(0, 1, 0));
			camera->lookAt(cameraEye, cameraEye + camera_direction*100.0, camera_up);
            float fovy = 60;
            camera->setPerspective(fovy, float(w) / float(h), 0.01f, 100);

           // loadModels();
            loadRayCastedModels();
        }

        void loadRayCastedModels()
        {
            //use script to load models
            scene->LoadSceneFromConfig("model/earth_plane/RayCastedGlobe.json");
            scene_weather->LoadSceneFromConfig("model/earth_plane/plane_scene.json");
            //fbcManager_->fbc_->_mesh->createSceneNode(scene_fengbaochao, "fengbaochao");
            fbcManager_->prepareFengBaoChaoData();
            fbcManager_->addFengBaoChaoDataToSceneManager(scene_fengbaochao, "fengbaochao");
        }

        void UpdateGUI()
        {
            ImGui::Begin("Params");
            auto& io = ImGui::GetIO();
            auto mean_framerate = io.Framerate;
            auto frametime = io.DeltaTime;
            ImGui::Text("%.3f ms/frame (%.1f FPS)", frametime, mean_framerate);
			//ImGui::Text("modified by Lancha");
            //update
            if (true) {
                Geodetic3D pos = earthshape->ToGeodetic3D(main_camera->position());
                double log = degrees(pos.getLongitude());
                double lat = degrees(pos.getLatitude());
                double hei = pos._height;
               // ImGui::Begin("Geometry Information");
                //auto& io = ImGui::GetIO();
                ImGui::Text("Longitude: %.2lf, Latitude: %.2lf, Height: %.2lf km", log, lat, hei/1000.0f);
				ImGui::Text("Clicked Position->Longitude: %.2lf, Latitude: %.2lf", degrees(globeInteractive->clicked_longtitude), degrees(globeInteractive->clicked_latitude));
               // ImGui::End();
            }
            ImGui::End();
        }
    };
}

