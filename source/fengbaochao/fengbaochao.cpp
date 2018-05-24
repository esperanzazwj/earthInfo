#include "fengbaochao.h"
#include "../as-is/Engine/OpenglDriver/GLGeometry.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <memory>
using namespace std;

	static bool loadFile(const string &file, double* tar)
	{
		ifstream fs(file.c_str(), ios::binary);
		fs.seekg(0, ios::end);
		int size = fs.tellg();
		if (size <= 0) return false;
		fs.seekg(0);
		fs.read((char*)tar, size);
		fs.close();
		return true;
	}

	static bool loadIntFile(const string &file,int* tar)
	{
		ifstream fs(file.c_str(), ios::binary);
		fs.seekg(0, ios::end);
		int size = fs.tellg();
		if (size <= 0) return false;
		fs.seekg(0);
		fs.read((char*)tar, size);
		fs.close();
		return true;
	}

    FengBaoChao::FengBaoChao(Ellipsoid * earthshape, MomentumCamera *mainCamera)
	{
		id = -1;
		_status = 0;
		_curType = 0;
		_mesh = NULL;
		_cutmesh = NULL;
		_loader_thread = false;
		b_cutterMaking = false;
		tempCol = NULL;
		b_cut = false;
        _earthshape = earthshape;
        _mainCamera = mainCamera;
		//new intializer
		speedMesh = NULL;
		speedDataPool.clear();
		speedDataEntity = NULL;
        tempMesh = NULL;
        tempDataEntity = NULL;
        pressMesh = NULL;
        pressDataPool.clear();
        pressDataEntity = NULL;
	}

	FengBaoChao::~FengBaoChao() {}

	//读取Speed数据到SpeedMeshes
	void FengBaoChao::initializeSpeedData()
	{
		for (int x = 0; x < 301; x++)
		{
			for (int y = 0; y < 201; y++)
			{
				HData[x][y] = 5000;
			}
		}

		//从文件中读取所有的speed数据到DataPool中，共speedDataNum个
		readAllSpeedDataIntoPool();

		//初始化SpeedMesh
		speedMesh = new Mesh<P3_C4, int>(VirtualGlobeRender::TRIANGLES, VirtualGlobeRender::GPU_DYNAMIC);
		speedMesh->setCapacity(numx*numy, (numx - 1)*(numy - 1) * 6);

		//设置索引
		int js = 0;
		for (int s = 0; s < 200; s++)
		{
			for (int k = 0; k < 300; k++)
			{
				speedMesh->setIndice(js, s * 301 + k + 301);
				speedMesh->setIndice(js + 1, s * 301 + k + 1);
				speedMesh->setIndice(js + 2, s * 301 + k + 0);
				speedMesh->setIndice(js + 3, s * 301 + k + 301);
				speedMesh->setIndice(js + 4, s * 301 + k + 302);
				speedMesh->setIndice(js + 5, s * 301 + k + 1);
				js = js + 6;
			}
		}

		for (int i = 0; i < numx; i++)
		{
			for (int j = 0; j < numy; j++)
			{
				vpat2[i][j] = 0;
			}
		}

		//设置顶点属性信息
		//updateSpeedData(0);
	}

	void FengBaoChao::readAllSpeedDataIntoPool()
	{
		speedDataPool.resize(speedDataNum);

		char pathNameU[512];
		char pathNameV[512];

		for (int k = 0; k < speedDataNum; k++)
		{
			sprintf(pathNameU, "runtime/taifeng/newVelocity/U10-%d.dat", k);
			sprintf(pathNameV, "runtime/taifeng/newVelocity/V10-%d.dat", k);
			double *import_data = new double[numx*numy * 2];

			if (!loadFile(pathNameU, import_data))
			{
				cout << "错误:数据读取错误" << endl;
			}
			if (!loadFile(pathNameV, import_data + numx * numy))
			{
				cout << "错误:数据读取错误" << endl;
			}
			speedDataPool[k] = import_data;
		}
	}

    //读取温度，压力数据
    void FengBaoChao::initializeTempData()
    {
        double* data = new double[numx*numy * 20];
        //这里原版读文件判断的是另一个文件pressure，读的是tv，奇怪
        if (loadFile("runtime/taifeng/TV.dat", data))
        {
            int idx = 0;
            for (int z = 0; z < 20; z++)
            {
                for (int y = 0; y < numy; y++)
                {
                    for (int x = 0; x < numx; x++)
                    {
                        PData[z][x][y] = data[idx++];
                    }
                }
            }

        }
        else
        {
            cout << "error (data read)" << endl;
        }
        delete[]data;

        //初始化TempMesh
        tempMesh = new Mesh<P3_C4, int>(VirtualGlobeRender::TRIANGLES, VirtualGlobeRender::GPU_DYNAMIC);
        tempMesh->setCapacity(numx*numy * 20, (numx - 1)*(numy - 1) * 6 * 20);


        //生成绘制索引数组
        int js = 0;
        for (int t = 0; t < 20; t++)
        {
            for (int s = 0; s < 200; s++)
            {
                for (int k = 0; k < 300; k++)
                {
                    int base = t * numx*numy;
                    tempMesh->setIndice(js, s * 301 + k + 301 + base);
                    tempMesh->setIndice(js + 1, s * 301 + k + 1 + base);
                    tempMesh->setIndice(js + 2, s * 301 + k + 0 + base);

                    tempMesh->setIndice(js + 3, s * 301 + k + 301 + base);
                    tempMesh->setIndice(js + 4, s * 301 + k + 302 + base);
                    tempMesh->setIndice(js + 5, s * 301 + k + 1 + base);

                    js = js + 6;
                }
            }
        }

        vec3f *vertexPos = new vec3f[numx * numy * 20];
        tempCol = new vec4f[numx*numy * 20];

        for (int z = 0; z < 20; z++)
        {
            for (int x = 0; x < numx; x++)
            {
                for (int y = 0; y < numy; y++)
                {
                    Geodetic3D llh = Geodetic3D(radians((x - 4) / 10.0f + 110), radians((y - 0) / 10.0f + 15), 3000 * z);
                    vec3d p = _earthshape->ToVector3D(llh);
                    vertexPos[z*numx*numy + y * numx + x] = vec3f(p.x, p.y, p.z);

                    //TV速度场着色设定
                    if (PData[z][x][y] < 0)// || x < 100)
                    {
                        tempCol[z*numx*numy + y * numx + x] = vec4f(0, 0, 0, 0);
                    }
                    else if (PData[z][x][y]> 20)
                    {
                        tempCol[z*numx*numy + y * numx + x] = vec4f(1.0, 0, 0, 0.25);
                    }
                    else
                    {
                        int yu = (int)(20 - PData[z][x][y]) / 5 + 1;
                        vec4f cv;
                        switch (yu)
                        {
                        case 1:
                            cv = vec4f(1.0, (20.0 - PData[z][x][y]) / 5.0, 0, 0.25);
                            break;
                        case 2:
                            cv = vec4f(1.0 - (float)(20 - PData[z][x][y] - 5) / 5.0, 1.0, 0, 0.25);
                            break;
                        case 3:
                            cv = vec4f(0, 1.0, (float)(20 - PData[z][x][y] - 10) / 5.0, 0.25);
                            break;
                        case 4:
                        case 5:
                            cv = vec4f(0, 1.0 - (float)(20 - PData[z][x][y] - 15) / 5.0, 1.0, 0.25);
                            break;
                        default:
                            cv = vec4f(0, 1.0, 1.0, 0.25);
                            break;
                        }
                        tempCol[z*numx*numy + y * numx + x] = cv;
                    }

                }
            }
        }
        for (int i = 0; i< numx*numy * 20; i++)
        {
            tempMesh->setVertex(i, P3_C4(vertexPos[i], tempCol[i]));
            tempMesh->position[i * 3] = vertexPos[i].x;
            tempMesh->position[i * 3 + 1] = vertexPos[i].y;
            tempMesh->position[i * 3 + 2] = vertexPos[i].z;
            tempMesh->color[i * 4] = tempCol[i].x;
            tempMesh->color[i * 4 + 1] = tempCol[i].y;
            tempMesh->color[i * 4 + 2] = tempCol[i].z;
            tempMesh->color[i * 4 + 3] = tempCol[i].w;
        }

        delete[]vertexPos;
        delete[]tempCol;
    }

    void FengBaoChao::initializePressData()
    {
        readAllPressDataIntoPool();
        int idx = 0;
        for (int x = 0; x < sizex; x++)
        {
            for (int y = 0; y < sizey; y++)
            {
                PressureData[x][y] = pressDataPool[0][idx];
                PressureData2[x][y] = pressDataPool[1][idx++];
            }
        }
        memcpy(PressureData1, PressureData, sizeof(PressureData));
        pressDataIdx = 2;

        pressMesh = new Mesh<P3_C4, int>(VirtualGlobeRender::TRIANGLES, VirtualGlobeRender::GPU_DYNAMIC);
        pressMesh->setCapacity(sizex*sizey, (sizex - 1)*(sizey - 1) * 6);

        int js = 0;
        for (int s = 0; s < sizey - 1; s++)
        {
            for (int k = 0; k < sizex - 1; k++)
            {
                pressMesh->setIndice(js, s * sizex + k + sizex);
                pressMesh->setIndice(js + 1, s * sizex + k + 1);
                pressMesh->setIndice(js + 2, s * sizex + k + 0);

                pressMesh->setIndice(js + 3, s * sizex + k + sizex);
                pressMesh->setIndice(js + 4, s * sizex + k + sizex + 1);
                pressMesh->setIndice(js + 5, s * sizex + k + 1);
                js = js + 6;
            }
        }
    }

    void FengBaoChao::readAllPressDataIntoPool()
    {
        pressDataPool.resize(pressDataNum);

        char pathName[512];

        for (int k = 0; k < pressDataNum; k++)
        {
            sprintf(pathName, "runtime/taifeng/pressure/700-%d.dat", k*6);
            int *data = new int[sizex* sizey];
            
            if (!loadIntFile(pathName, data))
            {
                cout << "error(data read)" << endl;
            }

            for (int i = 0;i < sizex*sizey;i++)
            {
               // cout << data[i]<< endl;
            }

            pressDataPool[k] = data;
        }
    }

	void FengBaoChao::updateSpeedData(int dataID)
	{

		//updatedata()

		//下面的代码都是从原始数据计算顶点属性数据
		double *dataInSpeedDataPool = speedDataPool[dataID];
		int idx = 0;
		for (int y = 0; y < numy; y++)
		{
			for (int x = 0; x < numx; x++)
			{
				vel[x][y][0] = dataInSpeedDataPool[idx];
				vel[x][y][1] = dataInSpeedDataPool[numx*numy + idx++];
			}
		}

		//HSV模型颜色映射之方向、速度大小
		for (int i = 0; i < numx; i++)
		{
			for (int j = 0; j < numy; j++)
			{
				HSV[i][j][0] = (float)(atan2(vel[i][j][1], vel[i][j][0]) * 180.0 / M_PI);
				if (HSV[i][j][0] < 0) 
					HSV[i][j][0] = HSV[i][j][0] + 360.0;
				int s1 = (int)(vel[i][j][1] * vel[i][j][1] + vel[i][j][0] * vel[i][j][0]);
				HSV[i][j][1] = (1.0f - (float)sqrt(s1 / 606.0f));
				//assert(HSV[i][j][0] >= 0 && HSV[i][j][0] <= 360);
				//assert(HSV[i][j][1] >= 0 && HSV[i][j][1] <= 1);
			}
		}

		//transformToKthFrame(0, 0);
		//test 
		for (int i = 0; i < 1; i++)
		{
			//transformToKthFrame(dataID, i);
		}
	}

    void FengBaoChao::updatePressData(int dataID)
    {
        memcpy(PressureData1, PressureData2, sizeof(PressureData2));
        int idx = 0;
        for (int x = 0;x < sizex;x++)
        {
            for (int y = 0;y < sizey;y++)
            {
                PressureData2[x][y] = pressDataPool[dataID][idx++];
            }
        }
        
    }

	void FengBaoChao::transformToKthFrame()
	{
		//trans()
        switch (_curType)
        {
        case Speed:
        {
            vec3f * vertexPos = new vec3f[numx*numy];
            vec4f *vertexCol = new vec4f[numx*numy];

            //噪声融合更新
            float ss;
            //use vpat2 and vel to update vpat1
            getDP();
            //use vpat1 and pat to update vpat2
            for (int i = 0; i < numx; i++)
            {
                for (int j = 0; j < numy; j++)
                {
                    Geodetic3D llh = Geodetic3D(radians((i - 4) / 10.0f + 110), radians((j - 0) / 10.0f + 15), (double)HData[i][j]);
                    vec3d p = _earthshape->ToVector3D(llh);
                    vertexPos[j * numx + i] = vec3f(p.x, p.y, p.z);
                    //有个问题，一个数据文件产生的数据是每一帧都要变？
                    ss = 0.9f * vpat1[i][j] + 0.1f * pat[i][j][iframe % 32];
                    vpat2[i][j] = (int)ss < 255 ? (int)ss : 255;
                    HSV[i][j][2] = vpat2[i][j] / 255.0f;
                    assert(HSV[i][j][2] >= 0 && HSV[i][j][2] <= 1);
                    //HSV[i][j][2] = 0.5;
                }
            }
            //copy vpat2 to vpat
            filter();
            HSV2RGB();

            //顶点颜色设定
            for (int i = 0; i < numx; i++)
            {
                for (int j = 0; j < numy; j++)
                {
                    vertexCol[j * numx + i] = vec4f(RGB[i][j][0], RGB[i][j][1], RGB[i][j][2], 0.9);
                }
            }

            //设置顶点属性
            for (int i = 0; i < numx*numy; i++)
            {
                speedMesh->setVertex(i, P3_C4(vertexPos[i], vertexCol[i]));
                speedMesh->position[i * 3] = vertexPos[i].x;
                speedMesh->position[i * 3 + 1] = vertexPos[i].y;
                speedMesh->position[i * 3 + 2] = vertexPos[i].z;
                speedMesh->color[i * 4] = vertexCol[i].x;
                speedMesh->color[i * 4 + 1] = vertexCol[i].y;
                speedMesh->color[i * 4 + 2] = vertexCol[i].z;
                speedMesh->color[i * 4 + 3] = vertexCol[i].w;
            }

            delete[]vertexPos;
            delete[]vertexCol;
            break;
        }
        case Temp:
            return;
        case Press:
        {
            vec3f* vertexPos = new vec3f[sizex * sizey];
            vec4f*  vertexCol = new vec4f[sizex*sizey];
            //中间时刻插值
            float temp;
            for (int x = 0; x < sizex; x = x + 1)
                for (int y = 0; y < sizey; y = y + 1)
                {
                    temp = ((32 - (iframe % 32)) * PressureData1[x][y] + (iframe % 32) * PressureData2[x][y]) / 32.0f;
                    PressureData[x][y] = (int)(temp + 0.5f);
                }

            for (int i = 0; i < sizex; i++)
                for (int j = 0; j < sizey; j++)
                {
                    Geodetic3D llh = Geodetic3D(radians((j - 154) / 10.0f + 137), radians((i - 140) / 10.0f + 23), (double)PressureData[i][j]);
                    vec3d p = _earthshape->ToVector3D(llh);
                    vertexPos[j * sizex + i] = vec3f(p.x, p.y, p.z);
                    if (PressureData[i][j] == 0 || PressureData[i][j] % 20 >5)
                        vertexCol[j * sizex + i] = vec4f(0, 0, 0, 0);
                    else
                    {
                        int yu = (int)(3250 - PressureData[i][j]) / 100 + 1;
                        vec4f cv;
                        switch (yu)
                        {
                        case 1:
                            cv = vec4f(1.0, (float)(3250 - PressureData[i][j]) * 255 / 25500.0, 0, 0.75);
                            break;
                        case 2:
                            cv = vec4f(1.0 - (float)(3250 - PressureData[i][j] - 100) * 255 / 25500.0, 1.0, 0, 0.75);
                            break;
                        case 3:
                            cv = vec4f(0, 1.0, (float)(3250 - PressureData[i][j] - 200) * 255 / 25500.0, 0.75);
                            break;
                        case 4:
                            cv = vec4f(0, 1.0 - (float)(3250 - PressureData[i][j] - 300) * 255 / 25500, 1.0, 0.75);
                            break;
                        }
                        vertexCol[j * sizex + i] = cv;
                    }

                }



            for (int i = 0; i< sizex*sizey; i++)
            {
                pressMesh->setVertex(i, P3_C4(vertexPos[i], vertexCol[i]));
                pressMesh->position[i * 3] = vertexPos[i].x;
                pressMesh->position[i * 3 + 1] = vertexPos[i].y;
                pressMesh->position[i * 3 + 2] = vertexPos[i].z;
                pressMesh->color[i * 4] = vertexCol[i].x;
                pressMesh->color[i * 4 + 1] = vertexCol[i].y;
                pressMesh->color[i * 4 + 2] = vertexCol[i].z;
                pressMesh->color[i * 4 + 3] = vertexCol[i].w;

            }

            delete[]vertexPos;
            delete[]vertexCol;
        }
            break;
        default:
            return;
        }
		
	}

	void FengBaoChao::updateGPUData()
	{
        switch (_curType)
        {
        case Speed:
        {
            auto entity = speedDataEntity;
            for (auto& y : entity->getMesh()->m_SubMeshList_as)
            {
                auto mesh = y.second;
                auto &currentGeo = mesh->renderable;
                assert(mesh->renderable != NULL);
                GLGeometry *geo = dynamic_cast<GLGeometry*>(mesh->renderable);
                assert(geo != NULL);
                const GL_GPUVertexData &vertexData = geo->getGLGPUVertexData();
                speedMesh->updateGPUVertexData(vertexData);
            }
            break;
        }
        case Temp:
            return;
        case Press:
        {
            auto entity = pressDataEntity;
            for (auto& y : entity->getMesh()->m_SubMeshList_as)
            {
                auto mesh = y.second;
                auto &currentGeo = mesh->renderable;
                assert(mesh->renderable != NULL);
                GLGeometry *geo = dynamic_cast<GLGeometry*>(mesh->renderable);
                assert(geo != NULL);
                const GL_GPUVertexData &vertexData = geo->getGLGPUVertexData();
                pressMesh->updateGPUVertexData(vertexData);
            }
            break;
        }
        default:
            return;
        }
	}

	void FengBaoChao::addPass(Pass *pass)
	{
		fbc_pass = pass;
	}

	void FengBaoChao::updatePass()
	{
		if (_status == 0)
		{
			return;
		}

		if (_status == 1)
		{
            switch (_curType)
            {
            case Speed:
                if (iframe % 32 == 0)
                {
                    updateSpeedData(speedDataIdx);
                    speedDataIdx = (speedDataIdx + 1) % speedDataNum;
                    cout << "update speed data" << endl;
                }

                iframe++;
                if (iframe > 1000000)iframe = 0;
                break;
            case Temp:
                break;
            case Press:
                if (iframe % 32 == 0)
                {
                    updatePressData(pressDataIdx);
                    pressDataIdx = (pressDataIdx + 1) % pressDataNum;
                    cout << "update pressure data" << endl;
                }

                iframe++;
                if (iframe > 1000000)iframe = 0;
                break;
            default:
                return;
            }
            
		}

		
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

		mat4f worldToCamera = _mainCamera->m_view_matrix();
		vec3d eyed = _mainCamera->reference_center();
		Vector3 eyef = Vector3(eyed.x, eyed.y, eyed.z);
		mat4f cameraToScreen = _mainCamera->m_absolute_projection_matrix();
		mat4f worldToScreen = cameraToScreen * worldToCamera;
		auto worldToScreenMatrix4 = mat4fToMatrix4(worldToScreen);
		fbc_pass->setProgramConstantData("og_modelViewPerspectiveMatrix", worldToScreenMatrix4.ptr(), "mat4", sizeof(Matrix4));
		fbc_pass->setProgramConstantData("u_cam", eyef.ptr(), "vec3", sizeof(Vector3));

        b_cut = false;
		if (!b_cut)
			fbc_pass->setProgramConstantData("func", Vector4(0, 0, 0, 0).ptr(), "vec4", sizeof(Vector4));
		else
			fbc_pass->setProgramConstantData("func", Vector4(func.x, func.y, func.z, func.w).ptr(), "vec4", sizeof(Vector4));
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	void FengBaoChao::createDataEntity(SceneManager* scene_manager, const string &name)
	{
        speedDataEntity = speedMesh->createEntity(scene_manager, "F_speed");
        tempDataEntity = tempMesh->createEntity(scene_manager, "F_temp");
        pressDataEntity = pressMesh->createEntity(scene_manager, "F_press");
	}

	void FengBaoChao::addDataToRenderQueue(RenderQueue & renderQueue)
	{
        Entity *entity = NULL;
        switch (_curType)
        {
        case Speed:
        {
            entity = speedDataEntity;
            break;
        }
        case Temp:
        {
            entity = tempDataEntity;
            break;
        }
        case Press:
        {
            entity = pressDataEntity;
            break;
        }
        default:
            return;

        }
        
		for (auto& y : entity->getMesh()->m_SubMeshList_as) {
			auto mesh = y.second;

			RenderQueueItem item;
			item.entity = entity;
			item.asMesh = mesh;
			auto& currentGeo = item.asMesh->renderable;
			if (currentGeo == NULL) {
				auto g = GlobalResourceManager::getInstance().m_GeometryFactory;
				currentGeo = g->create(item.asMesh->geometry, fbc_pass->mInputLayout);
			}
			renderQueue.push_back(item);
		}
	}

	void FengBaoChao::filter()
	{
		memcpy(vpat, vpat2, sizeof(vpat2));
	}
 
	//use vpat2 and vel to update vpat1?
	void FengBaoChao::getDP()
	{
		float vx, vy, r; //float dx, dy;
		float dmax = 1.0f;

		for (int i = 0; i < numx; i++)
		{
			for (int j = 0; j < numy; j++)
			{
				vpat1[i][j] = 0;
			}
		}

		for (int i = 0; i < numx; i++)
		{
			for (int j = 0; j < numy; j++)
			{
				//dx = i - numxd2;
				//dy = j - numyd2;
				//r = dx * dx + dy * dy;
				//if (r < 1) r = 1;
				//vx = numx * dx / r+2 ; vy = numy * dy / r;
				//vx = 1f; vy = 0;
				vx = (float)vel[i][j][0];
				vy = (float)vel[i][j][1];
				r = vx * vx + vy * vy;
				if (r > dmax * dmax)
				{
					r = (float)sqrt(r);
					vx *= dmax / r;
					vy *= dmax / r;
				}
				float tx, ty, lx, ly, lin;
				tx = i + vx; ty = j + vy;
				lx = tx - (int)tx; ly = ty - (int)ty;
				if ((int)tx + 1 < numx && (int)tx > 0 && (int)ty > 0 && (int)ty + 1 < numy)
				{
					lin = (1 - lx) * (1 - ly) * vpat2[i][j];
					vpat1[(int)tx][(int)ty] += (int)lin;
					lin = lx * ly * vpat2[i][j];
					vpat1[(int)tx + 1][(int)ty + 1] += (int)lin;
					lin = lx * (1 - ly) * vpat2[i][j];
					vpat1[(int)tx + 1][(int)ty] += (int)lin;
					lin = (1 - lx) * ly * vpat2[i][j];
					vpat1[(int)tx][(int)ty + 1] += (int)lin;
				}
			}
		}
	}

	//HSV模型转换为RGB分量
	void FengBaoChao::HSV2RGB()
	{
		for (int i = 0; i < numx; i = i + 1)
			for (int j = 0; j < numy; j = j + 1)
			{
				if ((int)HSV[i][j][1] == 0)
				{
					RGB[i][j][0] = HSV[i][j][2];
					RGB[i][j][1] = HSV[i][j][2];
					RGB[i][j][2] = HSV[i][j][2];
				}
				int H;
				H = (int)(HSV[i][j][0] / 60.0f);
				float f, p, q, t;
				f = HSV[i][j][0] / 60.0f - H;
				p = HSV[i][j][2] * (1 - HSV[i][j][1]);
				q = HSV[i][j][2] * (1 - HSV[i][j][1] * f);
				t = HSV[i][j][2] * (1 - HSV[i][j][1] * (1 - f));
				if (t < 0) t = 0;
				if (t > 1) t = 1;
				if (p < 0) p = 0;
				if (p > 1) p = 1;
				if (q < 0) q = 0;
				if (q > 1) q = 1;
				//if (HSV[i, j, 2] < 0) HSV[i, j, 2] = 0;
				//if (HSV[i, j, 2] > 1) HSV[i, j, 2] = 1;

				switch (H)
				{
				case 0:
					RGB[i][j][0] = HSV[i][j][2];
					RGB[i][j][1] = t;
					RGB[i][j][2] = p;
					break;
				case 1:
					RGB[i][j][0] = q;
					RGB[i][j][1] = HSV[i][j][2];
					RGB[i][j][2] = p;
					break;
				case 2:
					RGB[i][j][0] = p;
					RGB[i][j][1] = HSV[i][j][2];
					RGB[i][j][2] = t;
					break;
				case 3:
					RGB[i][ j][0] = p;
					RGB[i][j][1] = q;
					RGB[i][ j][2] = HSV[i][j][2];
					break;
				case 4:
					RGB[i][j][0] = t;
					RGB[i][j][1] = p;
					RGB[i][j][2] = HSV[i][j][2];
					break;
				default:
					RGB[i][j][0] = HSV[i][j][2];
					RGB[i][j][1] = p;
					RGB[i][j][2] = q;
					break;

				}
			}
	}

    //噪声序列生成
    void FengBaoChao::makePatterns()
    {
        int lut[256];
        int phase[numx][numy];
        int i, j, k, t;
        for (i = 0; i < 256; i++)
        {
            lut[i] = i < 63 ? 0 : 255;
        }
        for (i = 0; i < numx; i++)
        {
            for (j = 0; j < numy; j++)
            {
                int RandKey = rand() % 255;
                phase[i][j] = RandKey;
            }
        }
        for (k = 0; k < 32; k++)
        {
            t = k * 256 / 32;
            for (i = 0; i < numx; i++)
            {
                for (j = 0; j < numy; j++)
                {
                    pat[i][j][k] = lut[(t + phase[i][j]) % 255];//0 or 255
                }
            }
        }
    }
