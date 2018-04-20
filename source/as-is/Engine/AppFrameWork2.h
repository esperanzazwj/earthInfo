#pragma once
#include "../../third-party/imgui/imgui-all.hpp"
#include "Camera.h"
#include "CameraControl.h"
#include "mathlib.h"
#include "SceneNode.h"
#include "Entity.h"
#include "SceneManager.h"
#include "MeshManager.h"
#include "Camera.h"
#include "OctreeSceneManager.h"
#include "OpenglDriver/EGLRenderSystem.h"
#include "RenderTarget.h"
#include "OpenglDriver/EGLRenderWindow.h"
#include "TextureManager.h"
#include "EngineUtil.h"
#include "Pipeline.h"

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

class App {
public:
    int w = 1024;
    int h = 756;

    string name;
    Pipeline* pipeline;
    GlobalVariablePool * gp;
    RenderSystem * render;
    SceneManager * scene;
    MeshManager * meshMgr;
    RenderTarget * renderwindow;
    Camera* camera;
    Camera* lightCamera;
    CameraFPS cam_ctrl;

    virtual void Init() = 0;
    virtual void Render() {}
    virtual void CreateScene() {}
    virtual void InitPipeline()
    {
        if (pipeline != NULL)
        pipeline->Init();
    }

    void InitWrapper()
    {
        InitDevice();
        CreateScene();
        InitPipeline();
        cam_ctrl.Init(camera);
    }

    void loadModels() {
        scene->LoadSceneFromConfig("default_assets/sceneConfig/"+ name +".json");
    }

    void UpdateGUIWrapper()
    {
        ImGui::Begin("Main Panel");
        static bool showObjectControl = false;
        static bool showAppPanel = false;
        if (ImGui::Button("showAppPanel")) showAppPanel = !showAppPanel;
        if(showAppPanel)
            UpdateGUI();
        if (ImGui::Button("showObjectControl")) showObjectControl = !showObjectControl;
        if (showObjectControl)
        {
            ImGui::Begin("control");
            SceneNode* pRoot = scene->GetSceneRoot();
            vector<SceneNode*> nodes = pRoot->getAllChildNodes();
            for (auto node : nodes)
            {
                Vector3 trans = node->getLocalTranslation();
                Vector3 scale = node->getScale();
                float uniformScale = scale[0];
                string transTag = node->getName() + " trans";
                ImGui::DragFloat3(transTag.c_str(), &trans[0], 0.05f);
                string scaleTag = node->getName() + " scale";
                ImGui::DragFloat(scaleTag.c_str(), &uniformScale, 0.05f);
                node->setTranslation(trans);
                node->setScale(Vector3(uniformScale));
            }

            ImGui::End();
        }
        ImGui::End();
    }
    virtual void UpdateGUI() {
    }

    SceneNode* LoadMeshtoSceneNode(string pfile, string name) {
        MeshPtr mesh = meshMgr->loadMesh_assimp_check(name, pfile);
        if (mesh==NULL) return NULL;
        Entity* entity = scene->getEntity(name);
        if(entity ==NULL)
            entity = scene->CreateEntity(name);
        entity->setMesh(mesh);
        SceneNode* snode = scene->getSceneNode(name);
        if (snode == NULL) {
            snode = scene->CreateSceneNode(name);
            snode->attachMovable(entity);
        }
        return snode;
    }

    void InitDevice()
    {
        meshMgr = MeshManager::getSingletonPtr();
        InitDefaultResources();
    }

    void InitDefaultResources() {
        // init texture settings;
        auto& tm = TextureManager::getInstance();
        tm.LoadTexturePreset("texture_preset.json");

        // init rendertarget settings;
        RenderTargetManager::getInstance().LoadRenderTargetPreset("RenderTarget_preset.json");
        renderwindow = new EGLRenderWindow("MainWindow", render);
        auto& rtm = RenderTargetManager::getInstance();
        rtm.add(renderwindow);

        //get ScreenQuad
        meshMgr->loadMesh_assimp_check("screenquad", "default_assets/ScreenQuad.obj");
    }

    void ShowControlPanel()
    {
        SceneNode* s = scene->GetSceneRoot();
        for (auto node : s->getAllChildNodes())
        {
            Vector3 trans = node->getTranslation();
            string transTag = node->getName() + " trans";
            ImGui::DragFloat3(transTag.c_str(), &trans[0], 0.05f);
            node->setTranslation(trans);

            Vector3 scale = node->getScale();
            string scaleTag = node->getName() + " scale";
            ImGui::DragFloat3(scaleTag.c_str(), &scale[0], 0.05f);
            node->setScale(scale);

        }
    }
};

