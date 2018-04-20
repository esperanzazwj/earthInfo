#pragma once
#include "../Engine/AppFrameWork2.h"
#include "../Engine/Pipeline/GlobePipeline.h"
#include "../../oceanInfo/Geometry/Ellipsoid.h"
#include "../../oceanInfo/Camera/worldCamera.h"

namespace app
{
    struct Globe: App
    {
        float earthRadius;
        VirtualGlobeCore::Ellipsoid *earthshape; // 地球数学模型
        VirtualGlobeScene::MomentumCamera *main_camera; // 主相机

        void Init()
        {
            name = "globe";
            render = new EGLRenderSystem;
            render->GetWandH(w, h);     // Reset framebuffer size because it may be different than what you set.
            render->Initialize();
            //initialize
            earthRadius = 6378137.0;
            earthshape = new VirtualGlobeCore::Ellipsoid(earthRadius, earthRadius, earthRadius);//should release
            main_camera = new VirtualGlobeScene::MomentumCamera(earthshape);//should release
            main_camera->set_viewport(vec4i(0, 0, w, h));
            InitWrapper();
        }

        void InitPipeline()
        {
            pipeline = new GlobePipeline(main_camera, earthshape, earthRadius);
            pipeline->Init();
        }

        void Render()
        {
            UpdateGUI();
            pipeline->Render();
        }

        void CreateScene()
        {
            scene = new OctreeSceneManager("scene1", render);
            SceneContainer::getInstance().add(scene);

			Vector3 cameraEye = Vector3(0, 0, 6378137.0 * 5);
			Vector3 camera_direction = Vector3(0, 0, -1);
			Vector3 camera_up = Vector3(0, 1, 0);
            camera = scene->CreateCamera("main");
			
           // camera->lookAt(Vector3(0, 0, 2.5), Vector3(0, 0, 0), Vector3(0, 1, 0));
			camera->lookAt(cameraEye, cameraEye + camera_direction*100.0, camera_up);
            float fovy = 45;
            camera->setPerspective(fovy, float(w) / float(h), 0.01f, 100);

           // loadModels();
            loadRayCastedModels();
        }

        void loadRayCastedModels()
        {
            scene->LoadSceneFromConfig("model/earth_plane/RayCastedGlobe.json");
        }

        void UpdateGUI()
        {
            ImGui::Begin("Params");
            auto& io = ImGui::GetIO();
            auto mean_framerate = io.Framerate;
            auto frametime = io.DeltaTime;
            ImGui::Text("%.3f ms/frame (%.1f FPS)", frametime, mean_framerate);
			//ImGui::Text("modified by Lancha");
            ImGui::End();
        }
    };
}

