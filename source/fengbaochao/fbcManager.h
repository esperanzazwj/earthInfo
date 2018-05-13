#pragma once

#include "../../fengbaochao/fengbaochao.h"
#include "../../fengbaochao/Mesh.h"
#include "../../fengbaochao/Types.h"

class fbcManager {
public:
    int val;
    FengBaoChao * fbc_;
    //SceneManager * scene_fengbaochao;

    fbcManager(SceneManager* scene_fengbaochao, Ellipsoid * earthshape, MomentumCamera* maincamera)
        //:scene_fengbaochao(scene_fengbaochao)
    {
        val = 1;
        fbc_ = new FengBaoChao(earthshape, maincamera);
        Init();
        //test
        //fbc_->_mesh->createSceneNode(scene_fengbaochao, "fengbaochao");
    }
    ~fbcManager() {}

    void LoadMesh()
    {
        fbc_->_status = 1;
        srand((unsigned)time(NULL));
        fbc_->makePatterns();
        fbc_->_mesh = new Mesh<P3_C4, int>(VirtualGlobeRender::TRIANGLES, VirtualGlobeRender::GPU_DYNAMIC);
        fbc_->_mesh->addAttributeType(0, 3, VirtualGlobeRender::A32F, false);
        fbc_->_mesh->addAttributeType(1, 4, VirtualGlobeRender::A32F, false);
    }
    void Init() {
        LoadMesh();
        switch (val)
        {
            case 1://speed
            {
                fbc_->_mesh->setCapacity(numx*numy, (numx - 1)*(numy - 1) * 6);
            }
            break;
            case 2://Temp
            {
                fbc_->_mesh->setCapacity(numx*numy * 20, (numx - 1)*(numy - 1) * 6 * 20);
            }
            break;
            case 3://press
            {
                fbc_->_mesh->setCapacity(sizex*sizey, (sizex - 1)*(sizey - 1) * 6);
            }
            break;
            default:
                break;
        }

        fbc_->initializeTestMesh();

        //int size;
        //fbc_->fbc_pass = PassManager::getInstance().LoadPass("fbc_pass", "fengbaochao/fengbaochao.json");

        //unsigned char* shader0 = loadShader("VG/scene/ColorModelNodeVS.glsl", size);
        //unsigned char* shader1 = loadShader("VG/scene/ColorModelNodeFS.glsl", size);
        //fbc_->_program = new Program(shaders);
       // fbc_->viewportTansMatrix = fbc_->_program->getUniformMatrix4f("og_modelViewPerspectiveMatrix");
       // fbc_->funcFS = fbc_->_program->getUniform4f("func");
      //  fbc_->funcFS->set(vec4f(0, 0, 0, 0));
        fbc_->iframe = 0;
        /*auto& rt_mgr = RenderTargetManager::getInstance();
        auto mainwindow = rt_mgr.get("MainWindow");
        fbc_->fbc_pass->renderTarget = mainwindow;*/
        //fbc_->fb = FrameBuffer::getDefault();

        // ready to run-> run work thread(rendering)
       /* struct wrapper
        {
            static int	thread_wrapper0(void* loader)
            {
                return ((FengBaoChaoPlugin*)loader)->fbc_->workThread();
            }
        };*/
        //fbc_->_loader_thread = true;
        //fbc_->_loaderThread = SDL_CreateThread(wrapper::thread_wrapper0, this);
        //fbc_->_loaderMutex = SDL_CreateMutex();
    }

    void enableSpeed(bool val)
    {
        cout << "#绘制速度场" << endl;
        switchTaifengType(1);
        //here you should ask engine to draw fengbaogao(call fengbaochao::draw)
    }

    void enableTemp(bool val)
    {
        cout << "#绘制temp" << endl;
        switchTaifengType(2);
    }

    void enablePress(bool val)
    {
        cout << "#绘制press" << endl;
        switchTaifengType(3);
    }

    void switchTaifengType(int val) {
        //fbc->_status==0 -->stop
        if (val != fbc_->_curType && fbc_->_status != 0) {
            cout << "Warning:请先停止当前的台风类型" << endl;
            return;
        }
        fbc_->_curType = val;
        if (0 == fbc_->_status) {
            Init();
        }
        else if (fbc_->_status == 1)
        {
            fbc_->_status = 2;
        }
        else if (fbc_->_status == 2) {
            fbc_->_status = 1;
        }
    }
};
