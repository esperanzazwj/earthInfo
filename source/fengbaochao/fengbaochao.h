/** @file xiamen_grid_node.h
 *  @brief 临时绘制一条船的类
 *  @author skillzero 
 */

#ifndef _FengBaoChao_H_
#define _FengBaoChao_H_

//#include "namespace.h"
//#include "render_view.h"

//#include "ork/core/GlEnv.h"
//#include "ork/resource/XMLResourceLoader.h"
//#include "ork/resource/ResourceManager.h"
//#include "ork/render/FrameBuffer.h"
//#include "ork/core/Timer.h"
//#include "ork/core/FileLogger.h"
//using namespace VirtualGlobeRender;

#include "../oceanInfo/Camera/worldCamera.h"
//#include "Objects/ModelNode.h"
//#include "Objects/manualObject.h"
//#include "scene/scene_mgr.h"
using namespace VirtualGlobeScene;

#include "../oceanInfo/Geometry/Ellipsoid.h"
#include "../as-is/Engine/pass.h"
#include "Mesh.h"

using namespace VirtualGlobeCore;

#include <vector>
#include <list>
#include <map>
//#include <QtWidgets/qmessagebox.h>
//#include "SDL.h"

#define numx 301
#define numy 201
#define sizex 280
#define sizey 309

//class oceaninfo::platform::RenderView;
//using namespace oceaninfo::platform;

class FengBaoChao
{
public:
	enum taifengType{
		Speed=1,
		Temp=2,
		Press=3
	};

	FengBaoChao(Ellipsoid * earthshape, MomentumCamera* maincamera);
	~FengBaoChao();

	void makePatterns();
	void initializeTestMesh();

	int id;
	int pat[numx][numy][32];
	int vpat[numx][numy];
	int vpat1[numx][numy];
	int vpat2[numx][numy];
	int HData[numx][numy];
	double PData[20][numx][numy];
	double vel[numx][numy][2];
	float HSV[numx][numy][3];
	float RGB[numx][numy][3];
	int PressureData[sizex][sizey];
	int PressureData1[sizex][sizey];
	int PressureData2[sizex][sizey];

	Mesh<P3_C4 ,int > *_mesh;
	Mesh<P3_C4 ,short > *_cutmesh;
    Pass* fbc_pass{};
	//ptr<Program> _program;
	//ptr<UniformMatrix4f> viewportTansMatrix;
	//ptr<Uniform4f> funcFS;
	//ptr<FrameBuffer> fb;
	int iframe;
	int _curType;
	int _status;

    Ellipsoid * _earthshape;
    MomentumCamera *_mainCamera;

	vec3d cutPoints[2];
	int cutCurIdx;
	bool b_cutterMaking;
	vec4f func;
	bool b_cut;

	//RenderView *render_view_;

	// work thread stuff
	//SDL_Thread *_loaderThread;
	//SDL_mutex *_loaderMutex;
	bool _loader_thread;
	map<int, double*> _dataPool;
	int workThread();
	void cut();
	void updateContent(float timeSinceLastTime);
	void drawContentAfterWater(float timeSinceLastTime);
	void filter();
	void getDP();
	void HSV2RGB();
	void stop();
	void tran();
	void updatedata();

	void update();

private:
	vec4f* tempCol;	

	//ptr<TextureRectangle> frameTex;
	//ptr<TextureRectangle> depthTex;
};

#endif
