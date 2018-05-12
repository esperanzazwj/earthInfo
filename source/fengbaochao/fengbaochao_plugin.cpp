/**
 * @file xiamen_grid_plugin.cpp
 *
 * @breif Impl. of XiamenGridPlugin
 */
#include "fengbaochao_plugin.h"

#include <functional>

#include <QtDebug>
#include <QtGui/QtGui>
#include <QtWidgets/qmessagebox.h>

#include "html_ui_view.h"
#include "qt_helper.h"
#include "render_view.h"
#include "scene/scene_mgr.h"
#include "fengbaochao.h"
#include "time.h"

using namespace std;

USING_MAIN_NAMESPACE
USING_PLUGIN_NAMESPACE

static unsigned char* loadShader(const string &file, int &size)
{
	ifstream fs(file.c_str(), ios::binary);
	fs.seekg(0, ios::end);
	size = fs.tellg();
	char *data = new char[size + 1];
	fs.seekg(0);
	fs.read(data, size);
	fs.close();
	data[size] = 0;
	return (unsigned char*) data;
}

FengBaoChaoPlugin::FengBaoChaoPlugin() : PluginBase("fengbaochao")
{
    toggle_disp_["speed_prop"] = std::bind(&FengBaoChaoPlugin::OnSpeedBtn, this,
            std::placeholders::_1);
	toggle_disp_["temp_prop"] = std::bind(&FengBaoChaoPlugin::OnTempBtn, this,
		std::placeholders::_1);
	toggle_disp_["press_prop"] = std::bind(&FengBaoChaoPlugin::OnPressBtn, this,
		std::placeholders::_1);
	toggle_disp_["cancel_prop"] = std::bind(&FengBaoChaoPlugin::OnCancelBtn, this,
		std::placeholders::_1);
	toggle_disp_["cut_prop"] = std::bind(&FengBaoChaoPlugin::OnCutBtn, this,
		std::placeholders::_1);
	toggle_disp_["recover_prop"] = std::bind(&FengBaoChaoPlugin::OnRecoverBtn, this,
		std::placeholders::_1);
	fbc_ = new FengBaoChao();
	mn_ = new ManualNode();
	node_loaded_ = false;
	on_cut_ = false;
	lbtn = rbtn = false;
}

FengBaoChaoPlugin::~FengBaoChaoPlugin()
{
	Unload(); 
}

void FengBaoChaoPlugin::mousePressEvent(QMouseEvent *evt){
	mouse_x = evt->x();
	mouse_y = evt->y();
	if(evt->button() == Qt::LeftButton)
		lbtn = true;
	else if(evt->button() == Qt::RightButton)
		rbtn = true;
}

void FengBaoChaoPlugin::mouseReleaseEvent(QMouseEvent *evt){
	mouse_x = evt->x();
	mouse_y = evt->y();

	if(fbc_->b_cutterMaking && fbc_->_curType == 2){	//Temp
		if(evt->button() == Qt::LeftButton)
		{
			vec3d point;
			render_view_->scene_manager()->main_framebuffer->setReadBuffer(DEPTH);

			float data = 1.0f;
			render_view_->scene_manager()->main_framebuffer->readPixels(evt->x(),
				render_view_->height()- evt->y()-1,1,1,DEPTH_COMPONENT,
				PixelType::FLOAT,Buffer::Parameters(),CPUBuffer(&data));
			if (data > 0.999999f)
				return;
			else
			{
				point = render_view_->getCamera()->unproject(
					vec3d(mouse_x,render_view_->height()-mouse_y-1,data));
			}
			fbc_->cutPoints[(fbc_->cutCurIdx)++] = point;
			if(fbc_->cutCurIdx ==2)
			{
				fbc_->b_cutterMaking = false;
				Geodetic3D fPos = render_view_->scene_manager()->earthshape->ToGeodetic3D(
					fbc_->cutPoints[0]) ;
				Geodetic3D lPos = render_view_->scene_manager()->earthshape->ToGeodetic3D(
					fbc_->cutPoints[1]) ;
				double longmid = (fPos.getLongitude() + lPos.getLongitude())/2.0f;
				double latmid =  (fPos.getLatitude() + lPos.getLatitude())/2.0f;
				//Geodetic3D mPos = Geodetic3D(longmid, latmid, 0);
				//vec3d midPos = mSZWidget->earthShape->ToVector3D(mPos);
				//使用地心做中心点，这样切面一定向上
				vec3d midPos = vec3d(0,0,0);

				//确定平面方程式
				vec3d abDir = (fbc_->cutPoints[0] - midPos).normalize();
				vec3d bcDir = (fbc_->cutPoints[1] - midPos).normalize();
				vec3d norDir = abDir.crossProduct(bcDir);
				norDir = norDir.normalize();
				fbc_->func.x= norDir.x;
				fbc_->func.y = norDir.y;
				fbc_->func.z = norDir.z;
				fbc_->func.w = -fbc_->func.x* midPos.x - fbc_->func.y*midPos.y - 
					fbc_->func.z*midPos.z;
				fbc_->funcFS->set(fbc_->func);

				//mSZWidget->b_pluginTakenControl = false;

				Geodetic3D llh0 = Geodetic3D(radians((- 4) / 10.0f + 110), radians((0) / 10.0f + 15) , 30000);
				vec3d p0 = render_view_->scene_manager()->earthshape->ToVector3D(llh0);

				Geodetic3D llh1 = Geodetic3D(radians((- 4) / 10.0f + 110), radians( numy / 10.0f + 15) , 30000);
				vec3d p1 = render_view_->scene_manager()->earthshape->ToVector3D(llh1);

				Geodetic3D llh2 = Geodetic3D(radians((numx- 4) / 10.0f + 110), radians((0) / 10.0f + 15) , 30000);
				vec3d p2 = render_view_->scene_manager()->earthshape->ToVector3D(llh2);

				Geodetic3D llh3 = Geodetic3D(radians((numx - 4) / 10.0f + 110), radians( numy/ 10.0f + 15) , 30000);
				vec3d p3 = render_view_->scene_manager()->earthshape->ToVector3D(llh3);
				//先写死了裂口只能在纵向的两个线上
				vec3d dir0,dir1,cross0,cross1;
				vec3d coner[4];
				Geodetic3D conerll[4];
				dir0 = (p1-p0).normalize();
				float d = norDir.dotproduct(dir0);
				float dist = -(( norDir.dotproduct(p0) + fbc_->func.w) )/ d;
				if (dist > 0)
				{
					cross0 = p0 + dir0 * dist;
					Geodetic3D lldcross = render_view_->scene_manager()->earthshape->ToGeodetic3D(cross0);
					double logt = lldcross.getLongitude();
					double lat = lldcross.getLatitude();
					conerll[0] = Geodetic3D(logt,lat,0);
					conerll[1] = Geodetic3D(logt,lat,3000*20);
					coner[0] = render_view_->scene_manager()->earthshape->ToVector3D(conerll[0]);
					coner[1] = render_view_->scene_manager()->earthshape->ToVector3D(conerll[1]);
				}
				else return;
				dir1 = (p3-p2).normalize();
				d = norDir.dotproduct(dir1);
				dist = -(( norDir.dotproduct(p2) + fbc_->func.w) )/ d;
				if (dist > 0)
				{
					cross1 = p2 + dir1 * dist;
					Geodetic3D lldcross = render_view_->scene_manager()->earthshape->ToGeodetic3D(cross1);
					double logt = lldcross.getLongitude();
					double lat = lldcross.getLatitude();
					conerll[2] = Geodetic3D(logt,lat,0);
					conerll[3] = Geodetic3D(logt,lat,3000*20);
					coner[2] = render_view_->scene_manager()->earthshape->ToVector3D(conerll[2]);
					coner[3] = render_view_->scene_manager()->earthshape->ToVector3D(conerll[3]);
				}
				else return;
				fbc_->_cutmesh = new Mesh<P3_C4, short>(TRIANGLES, GPU_DYNAMIC);
				fbc_->_cutmesh->addAttributeType(0, 3, A32F, false);
				fbc_->_cutmesh->addAttributeType(1, 4, A32F, false);
				fbc_->_cutmesh->setCapacity(numx*20, (numx-1)*6*19);
				double logStep = (conerll[3].getLongitude() - conerll[1].getLongitude())/(numx-1);
				double latStep = (conerll[3].getLatitude() - conerll[1].getLatitude())/(numx-1);
				double oriLog = conerll[1].getLongitude();
				double oriLat = conerll[1].getLatitude();
				int count =0;
				for (int z =0; z< 20; z++)
				{
					for (int i = 0; i< numx; i++)
					{
						Geodetic3D p = Geodetic3D(oriLog+i*logStep, oriLat+i*latStep, z*9000);
						vec3d pt = render_view_->scene_manager()->earthshape->ToVector3D(p);
						int h =(oriLat+ i*latStep - radians(15.0))/radians( numy/ 10.0f)*numy;
						vec4f col = vec4f(1,1,1,0.75);//tempCol[z*numx*numy + h*numx + i];
						col.w =0.6;
						fbc_->_cutmesh->setVertex(count++, P3_C4(vec3f(pt.x, pt.y,pt.z), col));
					}
				}
				count =0;
				for (int z =0; z<19; z++)
				{
					for (int i =0; i< numx-1; i++)
					{
						fbc_->_cutmesh->setIndice(count++, z*numx + i);
						fbc_->_cutmesh->setIndice(count++, z*numx + i+1);
						fbc_->_cutmesh->setIndice(count++, z*numx + i + numx);

						fbc_->_cutmesh->setIndice(count++, z*numx + i+1);
						fbc_->_cutmesh->setIndice(count++, z*numx + i+numx +1);
						fbc_->_cutmesh->setIndice(count++, z*numx + i + numx);
					}
				}
			}
		}		
	}
}

void FengBaoChaoPlugin::Load(const QDir& plugins_dir, RenderView* render_view)
{
    if (!loaded_) {
        PluginBase::Load(plugins_dir, render_view);
        //TODO
        loaded_ = true;
		render_view_ = render_view;
		if(fbc_ == NULL)
			fbc_ = new FengBaoChao();
		if(mn_ == NULL)
			mn_ = new ManualNode();
		fbc_->render_view_ = render_view;
		mn_->fbc = fbc_;
    }
}

void FengBaoChaoPlugin::Unload()
{
	if(node_loaded_ && render_view_ && render_view_->scene_manager())
		render_view_->scene_manager()->removeSceneNode(name_);
	if(fbc_)
		fbc_->stop();
    if (loaded_) {
        //TODO
        loaded_ = false;		
		node_loaded_ = false;
		if(fbc_)
		{
			delete fbc_;
			fbc_ = NULL;
		}
		if(mn_)
		{
			delete mn_;
			mn_ = NULL;
		}
    }
}

std::string FengBaoChaoPlugin::OnJsRequest(const std::string& message)
{
	qDebug() << "# FengBaoChaoPlugin::OnJsRequest, with message \"" << message.c_str() << "\"";
	//TODO
	QJsonObject request = FromJson(message);
	std::string type = request["type"].toString().toStdString();
	if (type == "Toggle")
	{
		std::string func = request["property"].toString().toStdString();
		std::string val = request["value"].toString().toStdString();
		bool fval = false;
		if(val == "1")
			fval = true;
		toggle_disp_.dispatch(func)(fval);
	}
	return "";
}

void FengBaoChaoPlugin::switchTaifengType(int val){
	if(val != fbc_->_curType && fbc_->_status != 0){
		QMessageBox::information(NULL,QStringLiteral("提示"),
			QStringLiteral("请先停止当前的台风类型"));
		return;
	}
	fbc_->_curType = val;
	if(0 == fbc_->_status){
		fbc_->_status = 1;
		srand((unsigned)time(NULL));
		fbc_->makePatterns();
		fbc_->_mesh = new Mesh<P3_C4, int>(TRIANGLES, GPU_DYNAMIC);
		fbc_->_mesh->addAttributeType(0, 3, A32F, false);
		fbc_->_mesh->addAttributeType(1, 4, A32F, false);

		switch(val){
		case 1://speed
			{
				fbc_->_mesh->setCapacity(numx*numy, (numx-1)*(numy-1)*6);
			}
			break;
		case 2://Temp
			{
				fbc_->_mesh->setCapacity(numx*numy*20, (numx-1)*(numy-1)*6*20);
			}
			break;
		case 3://press
			{
				fbc_->_mesh->setCapacity(sizex*sizey, (sizex-1)*(sizey-1)*6);
			}
			break;
		default:
			break;
		}

		fbc_->initializeTestMesh();

		int size;
		unsigned char* shader0 = loadShader("VG/scene/ColorModelNodeVS.glsl", size);
		unsigned char* shader1 = loadShader("VG/scene/ColorModelNodeFS.glsl", size);
		ptr<Module> shaders = new Module(330, (char*)shader0 , (char*)shader1 );
		fbc_->_program = new Program(shaders);
		fbc_->viewportTansMatrix = fbc_->_program->getUniformMatrix4f("og_modelViewPerspectiveMatrix");
		fbc_->funcFS = fbc_->_program->getUniform4f("func");
		fbc_->funcFS->set(vec4f(0,0,0,0));
		fbc_->iframe =0;
		fbc_->fb = FrameBuffer::getDefault();
		delete []shader0;
		delete []shader1;
		struct wrapper 
		{
			static int	thread_wrapper0(void* loader)
			{
				return ((FengBaoChaoPlugin*) loader)->fbc_->workThread();
			}
		};
		fbc_->_loader_thread = true;
		fbc_->_loaderThread = SDL_CreateThread(wrapper::thread_wrapper0, this);
		fbc_->_loaderMutex = SDL_CreateMutex();
	}
	else if (fbc_->_status == 1)
	{
		fbc_->_status = 2;
	}
	else if(fbc_->_status == 2){
		fbc_->_status = 1;
	}
}

void FengBaoChaoPlugin::OnSpeedBtn(bool val)
{
	qDebug()<<"#绘制速度场";
	switchTaifengType(1);
	if(!node_loaded_)
	{
		render_view_->scene_manager()->addSceneNode(name_,mn_);	
		node_loaded_ = true;
	}
}

void FengBaoChaoPlugin::OnTempBtn(bool val)
{
	switchTaifengType(2);
	if(!node_loaded_)
	{
		render_view_->scene_manager()->addSceneNode(name_,mn_);	
		node_loaded_ = true;
	}	
}

void FengBaoChaoPlugin::OnPressBtn(bool val)
{
	switchTaifengType(3);
	if(!node_loaded_)
	{
		render_view_->scene_manager()->addSceneNode(name_,mn_);	
		node_loaded_ = true;
	}	
}

void FengBaoChaoPlugin::OnCancelBtn(bool val)
{
	if(node_loaded_)
	{
		fbc_->stop();
		render_view_->scene_manager()->removeSceneNode(name_);
		node_loaded_ = false;
	}
}

void FengBaoChaoPlugin::OnCutBtn(bool val)
{
	if(!node_loaded_ || fbc_->_curType != 2){
		fbc_->b_cutterMaking = false;
		render_view_->setActionHandlePlugin(NULL);
		action_handled_ = false;
		return;
	}
	fbc_->b_cutterMaking = true;
	if(fbc_->b_cutterMaking){
		render_view_->setActionHandlePlugin(this);
		action_handled_ = true;
		fbc_->cutCurIdx=0;
	}
	else{
		render_view_->setActionHandlePlugin(NULL);
		action_handled_ = false;
	}
}

void FengBaoChaoPlugin::OnRecoverBtn(bool val)
{
	action_handled_ = false;
	if(node_loaded_){
		fbc_->b_cut = !fbc_->b_cut;
	}
}

void FengBaoChaoPlugin::update()
{
	if(!fbc_->b_cutterMaking)
		action_handled_ = false;
}
