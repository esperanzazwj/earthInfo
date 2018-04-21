#pragma once
#include "../Engine/AppFrameWork2.h"
#include "../Engine/Pipeline/GlobePipeline.h"
#include "../../oceanInfo/Geometry/Ellipsoid.h"
#include "../../oceanInfo/Camera/worldCamera.h"
#include "../Engine/CameraControl.h"
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
            //camera controller
            app::globeInteractive = new GlobeInteractive(main_camera, earthshape);

            InitWrapper();

            auto& ctx = ss::Window_System::current().context();
            glfwSetScrollCallback(ctx, app::scrollMoveEvent);
        }

        void InitPipeline()
        {
            pipeline = new GlobePipeline(main_camera, earthshape, earthRadius, app::globeInteractive);
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

			Vector3 cameraEye = Vector3(6378137.0 * 3, 0, 0);
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
            //update
            if (true) {
                Geodetic3D pos = earthshape->ToGeodetic3D(main_camera->position());
                double log = degrees(pos.getLongitude());
                double lat = degrees(pos.getLatitude());
                double hei = pos._height;
               // ImGui::Begin("Geometry Information");
                auto& io = ImGui::GetIO();
                ImGui::Text("Longitude: %.2lf, Latitude: %.2lf, Height: %.2lf km", log, lat, hei/1000.0f);
               // ImGui::End();
            }
            ImGui::End();
        }
    };
}

