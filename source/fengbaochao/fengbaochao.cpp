#include "fengbaochao.h"
//#include "Util.h"
#include <iostream>
#include <fstream>
#include <math.h>
//#include <QObject>
using namespace std;
//using namespace oceaninfo::platform;

	static bool loadFile(const string &file,double* tar)
	{
		ifstream fs(file.c_str(), ios::binary);
		fs.seekg(0, ios::end);
		int size = fs.tellg();
		if(size<= 0)
			return false;
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
		if(size<= 0)
			return false;
		fs.seekg(0);
		fs.read((char*)tar, size);
		fs.close();
		return true;
	}

    FengBaoChao::FengBaoChao(Ellipsoid * earthshape, MomentumCamera *mainCamera)
	{
		id = -1;
		_status = 0;
		_curType = -1;
		_mesh = NULL;
		_cutmesh = NULL;
		_loader_thread = false;
		b_cutterMaking = false;
		tempCol = NULL;
		b_cut = false;
        _earthshape = earthshape;
        _mainCamera = mainCamera;
	}

	FengBaoChao::~FengBaoChao()
	{
		//if(NULL != _loaderThread)
		//	delete _loaderThread;
		//if(NULL != _loaderMutex)
		//	delete _loaderMutex;
	}

	//读取初始时刻数据
	void FengBaoChao::initializeTestMesh()
	{
		switch(_curType)
		{
		case Speed:
			{
				for (int x = 0; x < 301; x++)
				{
					for (int y = 0; y < 201; y++)
					{
						HData[x][y] = 5000;
					}
				}

				double* data = new double[numx* numy];
				if (loadFile("VG/taifeng/newVelocity/U10-0.dat",data))
				{
					int idx =0;
					for (int y = 0; y < numy; y++)
					{
						for (int x = 0; x < numx; x++)
						{
							vel[x][y][0] = data[idx++];
						}
					}
				}
				else
                {
                    //QMessageBox::information(NULL,QStringLiteral("错误"),
                     //   QStringLiteral("数据读取错误"));
                    cout << "error(data read)" << endl;
                }

				if (loadFile("VG/taifeng/newVelocity/V10-0.dat",data))
				{
					int idx =0;
					for (int y = 0; y < numy; y++)
					{
						for (int x = 0; x < numx; x++)
						{
							vel[x][y][1] = data[idx++];
						}
					}
				}
				else
                {
                    //QMessageBox::information(NULL,QStringLiteral("错误"),
					//    QStringLiteral("数据读取错误"));
                    cout << "error(data read)" << endl;
                }
				delete []data;
				//SDL_LockMutex(_loaderMutex);//预读5个
				for (int i =1 ;i <6; i++)
				{
					_dataPool[i] = NULL;
				}
				//SDL_UnlockMutex(_loaderMutex);
				//此处是为了确定最大速度值w，以便进行颜色映射
				//int w = 606;
				//float sss, kkk;
				//sss = 0f; kkk = 0f;

				//HSV模型颜色映射之方向、速度大小
				for (int i = 0; i < numx; i++)
				{
					for (int j = 0; j < numy; j++)
					{
						HSV[i][j][0] = (float)(atan2(vel[i][j][1], vel[i][j][0]) * 180.0 / M_PI);
						if (HSV[i][j][0] < 0) HSV[i][j][0] = HSV[i][j][0] + 360.0;
						//double sk1,sk2;
						//sk1=vel[i, j, 1]+0.0;sk2=vel[i, j, 2]+0.0;
						//HSV[i, j, 0] =(float) Math.Atan2(sk2, sk1);
						int s1 = (int)(vel[i][j][1] * vel[i][j][1] + vel[i][j][0] * vel[i][j][0]);
						HSV[i][j][1] = (1.0f - (float)sqrt(s1 / 606.0f));
						//HSV[i, j, 1] = (float)Math.Sqrt(s1 / 977.0f);
						//if (sss > HSV[i, j, 0]) sss = HSV[i, j, 0];
						//if (kkk < HSV[i, j, 0]) kkk = HSV[i, j, 0];
					}
				}

				//生成绘制索引数组
				int js = 0;
				for (int s = 0; s < 200; s++)
				{
					for (int k = 0; k < 300; k++)
					{
						_mesh->setIndice(js, s * 301 + k + 301);
                        _mesh->setIndice(js + 1, s * 301 + k + 1);
                        _mesh->setIndice(js + 2, s * 301 + k + 0);

						_mesh->setIndice(js + 3, s * 301 + k + 301);
						_mesh->setIndice(js + 4, s * 301 + k + 302);
						_mesh->setIndice(js + 5, s * 301 + k + 1);
						js = js + 6;
					}
				}
			}
			break;
		case Temp:
			{
				double* data = new double[numx*numy*20];
				//这里原版读文件判断的是另一个文件pressure，读的是tv，奇怪
				if (loadFile("VG/taifeng/TV.dat",data))
				{
					int idx =0;
					for(int z=0; z<20; z++)
						for (int y = 0; y < numy; y++)
						{
							for (int x = 0; x < numx; x++)
							{
								PData[z][x][y] = data[idx++];
							}
						}
				}
				else { /*QMessageBox::information(NULL,QStringLiteral("错误"),
					QStringLiteral("数据读取错误"));*/
                    cout << "error (data read)" << endl;
                }
				delete []data;

				//生成绘制索引数组
				int js = 0;
				for (int t=0; t<20; t++)
				{
					for (int s = 0; s < 200; s++)
					{
						for (int k = 0; k < 300; k++)
						{
							int base = t*numx*numy;
							_mesh->setIndice(js, s * 301 + k + 301 + base);
							_mesh->setIndice(js +1, s * 301 + k + 1+ base);
							_mesh->setIndice(js+2,s * 301 + k + 0+ base);

							_mesh->setIndice(js + 3, s * 301 + k + 301+ base);
							_mesh->setIndice(js + 4, s * 301 + k + 302+ base);
							_mesh->setIndice(js + 5, s * 301 + k + 1+ base);

							js = js + 6;
						}
					}
				}

				vec3f* vertexPos = new vec3f[numx * numy*20];
				tempCol = new vec4f[numx*numy*20];

				for (int z = 0; z < 20; z++)
				{
					for (int x = 0; x < numx; x++)
					{
						for (int y = 0; y < numy; y++)
						{
							Geodetic3D llh = Geodetic3D(radians((x - 4) / 10.0f + 110), radians((y - 0) / 10.0f + 15) , 3000*z);
							vec3d p = _earthshape->ToVector3D(llh);
							vertexPos[z*numx*numy + y * numx + x] = vec3f(p.x, p.y, p.z);

							//TV速度场着色设定
							if(PData[z][x][y] < 0)// || x < 100)
							{
								tempCol[z*numx*numy + y * numx + x]= vec4f(0,0,0,0);
							}
							else if (PData[z][x][y]> 20)
							{
								tempCol[z*numx*numy + y * numx + x]= vec4f(1.0, 0, 0,127.0/255.0);
							}
							else
							{
								int yu = (int)(20 - PData[z][x][y]) / 5 + 1;
								vec4f cv;
								switch (yu)
								{
								case 1:
									cv = vec4f(1.0, (20.0 - PData[z][x][y]) / 5.0, 0,0.5);
									break;
								case 2:
									cv = vec4f(1.0 - (float)(20 - PData[z][x][y] - 5) / 5.0, 255, 0,0.5);
									break;
								case 3:
									cv = vec4f(0, 1.0, (float)(20 - PData[z][x][y] - 10) / 5.0,0.5);
									break;
								case 4:
								case 5:
									cv = vec4f(0, 1.0 - (float)(20 - PData[z][x][y] - 15)/ 5.0, 255,0.5);
									break;
								default:
									cv = vec4f(0, 1.0, 1.0,0.5);
									break;
								}
								tempCol[z*numx*numy + y * numx + x] = cv;
							}

						}
					}
				}
				for (int i =0; i< numx*numy*20; i++)
				{
					_mesh->setVertex(i, P3_C4(vertexPos[i], tempCol[i]));
				}
				delete []vertexPos;
			}
			break;
		case Press:
			{
				int *data = new int[sizex*sizey];
				if (loadIntFile("VG/taifeng/pressure/700-0.dat",data))
				{
					int idx =0;
					for (int x = 0; x < sizex; x++)
					{
						for (int y = 0; y < sizey; y++)
						{
							PressureData[x][y] = (int)data[idx++];
						}
					}
				}
				else { /*QMessageBox::information(NULL,QStringLiteral("错误"),
					QStringLiteral("数据读取错误"));*/
                    cout << "error(data read)" << endl;
                }
				memcpy(PressureData1, PressureData,sizeof(PressureData));

				if (loadIntFile("VG/taifeng/pressure/700-6.dat",data))
				{
					int idx =0;
					for (int x = 0; x < sizex; x++)
					{
						for (int y = 0; y < sizey; y++)
						{
							PressureData2[x][y] = (int)data[idx++];
						}
					}
				}
				else { /*QMessageBox::information(NULL,QStringLiteral("错误"),
					QStringLiteral("数据读取错误"));*/
                    cout << "error(data read)" << endl;
                }

				delete []data;
				//SDL_LockMutex(_loaderMutex);//预读3个
				for (int i =2 ;i <5; i++)
				{
					_dataPool[i*6] = NULL;
				}
				//SDL_UnlockMutex(_loaderMutex);
				//生成绘制索引数组
				int js = 0;
				for (int s = 0; s < sizey-1; s++)
				{
					for (int k = 0; k < sizex -1; k++)
					{

						_mesh->setIndice(js, s * sizex + k + sizex);
						_mesh->setIndice(js +1, s * sizex + k + 1);
						_mesh->setIndice(js+2,s * sizex + k + 0);

						_mesh->setIndice(js + 3, s * sizex + k + sizex);
						_mesh->setIndice(js + 4, s * sizex + k + sizex+1);
						_mesh->setIndice(js + 5, s * sizex + k + 1);
						js = js + 6;
					}
				}
			}
			break;
		default:
			return;
		}
	}
   
	void FengBaoChao::cut()
	{
		b_cutterMaking = true;
		cutCurIdx = 0;
	}

	void FengBaoChao::draw()
	{
		if(_mainCamera->csys() == CoordinateSystem::GLOBAL)
			drawContent(0);		
	}
   
	void FengBaoChao::drawContent(float timeSinceLastTime)
	{
	/*	if (_status == 0)
		{
			return;
		}
		if(_status == 1)
			iframe++;
		if(iframe %32 ==0 && _status ==1)
		{
			updatedata();
		}
		tran();

		mat4f worldToCamera = render_view_->getCamera()->m_view_matrix();
		vec3d eyed = render_view_->getCamera()->reference_center();
		vec3f eyef(eyed.x, eyed.y, eyed.z);
		mat4f cameraToScreen = render_view_->getCamera()->m_absolute_projection_matrix();
		viewportTansMatrix->setMatrix(cameraToScreen * worldToCamera);
		_program->getUniform3f("u_cam")->set(eyef);
		//fb->setDepthTest(false);
		fb->setBlend(true,ADD,SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
		if(!b_cut)
			funcFS->set(vec4f(0,0,0,0));
		else
			funcFS->set(func);
		fb->draw(_program, *_mesh);

		if(_cutmesh != NULL && !b_cut)
		{
			funcFS->set(vec4f(0,0,0,0));
			fb->draw(_program, *_cutmesh);
			funcFS->set(func);
		}
		fb->setBlend(false);*/
	}
   
	void FengBaoChao::drawContentAfterWater(float timeSinceLastTime)
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
   
	void FengBaoChao::filter()
	{
		memcpy(vpat, vpat2, sizeof(vpat2));
	}
 
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

	void FengBaoChao::stop()
	{
		//actStop->setCheckable(false);
		_status = 0;
		//setPluginStatus(UNACITVED);		
		_mesh = NULL;
		_cutmesh = NULL;
		//_program = NULL;
		//_loader_thread = false;
		//SDL_WaitThread(_loaderThread, NULL);
		//_loaderThread = NULL;
		//SDL_DestroyMutex(_loaderMutex);
		//_loaderMutex = NULL;
		map<int, double*>::iterator it = _dataPool.begin();
		while (it != _dataPool.end())
		{
			delete [] it->second;
			it++;
		}
		_dataPool.clear();
		if(tempCol)
		{
			delete []tempCol;
			tempCol = NULL;
		}
		b_cutterMaking = false;
		b_cut = false;
		func = vec4f(0,0,0,0);
		//mSZWidget->b_pluginTakenControl = false;
		//if(render_view_)
		//	render_view_->setActionHandlePlugin(NULL);
	}

	void FengBaoChao::tran()
	{
		switch(_curType)
		{
		case Speed:
			{
				vec3f* vertexPos = new vec3f[numx * numy];
				vec4f*  vertexCol = new vec4f[numx*numy];
				//噪声融合更新
				float ss;
				getDP();
				for (int i = 0; i < numx; i++)
					for (int j = 0; j < numy; j++)
					{
						Geodetic3D llh = Geodetic3D(radians((i - 4) / 10.0f + 110), radians((j - 0) / 10.0f + 15) , (double)HData[i][j]);
						vec3d p = _earthshape->ToVector3D(llh);
						vertexPos[j * numx + i] = vec3f(p.x, p.y, p.z);
						ss = 0.9f * vpat1[i][j] + 0.1f * pat[i][j][iframe % 32];
						//ss = 0.9f * ss + 0.1f * (255 * j / 308 );
						vpat2[i][j] = (int)ss < 255 ? (int)ss : 255;
						HSV[i][j][2] = vpat2[i][j] / 255.0f;
					}

					filter();
					HSV2RGB();
					//顶点颜色设定
					for (int i = 0; i < numx; i++)
						for (int j = 0; j < numy; j++)
						{
							vertexCol[j * numx + i] = vec4f(RGB[i][j][0] , RGB[i][j][1], RGB[i][j][2],150.0/255.0);


							//verts[i * numy + j].Normal= new Vector3(0f,0f,-1f);
							//vertexes1[i * numy + j].Color = Color.FromArgb(80, vpat[i, j], vpat[i, j], vpat[i, j]).ToArgb();
							//vertexes1[i * numy + j].Color = Color.FromArgb(200, (int)(RGB[i, j, 0] * 255.0f), (int)(RGB[i, j, 1] * 255.0f), (int)(RGB[i, j, 2] * 255.0f)).ToArgb();
						}
						for (int i =0; i< numx*numy; i++)
						{
							_mesh->setVertex(i, P3_C4(vertexPos[i], vertexCol[i]));
						}

						delete []vertexPos;
						delete []vertexCol;
			}
			break;
		case Temp:
			{
				return;
			}
		case Press:
			{
				vec3f* vertexPos = new vec3f[sizex * sizey];
				vec4f*  vertexCol = new vec4f[sizex*sizey];
				//中间时刻插值
				float temp;
				for (int x = 0; x < sizex; x = x + 1)
					for (int y = 0; y < sizey; y = y + 1)
					{
						temp = ((32 - (iframe % 32)) * PressureData1[x][y] + (iframe % 32) * PressureData2[x][y])/ 32.0f;
						PressureData[x][y] = (int)(temp + 0.5f);
					}

					for (int i = 0; i < sizex; i++)
						for (int j = 0; j < sizey; j++)
						{
							Geodetic3D llh = Geodetic3D(radians((j - 154) / 10.0f + 137), radians((i - 140) / 10.0f + 23) , (double)PressureData[i][j]);
							vec3d p = _earthshape->ToVector3D(llh);
							vertexPos[j * sizex + i] = vec3f(p.x, p.y, p.z);
							if(PressureData[i][j] ==0 || PressureData[i][j] %20 >5)
								vertexCol[j * sizex + i] = vec4f(0,0,0,0);
							else 
							{
								int yu = (int)(3250 - PressureData[i][j]) / 100 + 1;
								vec4f cv;
								switch (yu)
								{
								case 1:
									cv = vec4f(1.0, (float)(3250 - PressureData[i][j]) * 255 / 25500.0, 0,0.5);
									break;
								case 2:
									cv = vec4f(1.0 - (float)(3250 - PressureData[i][j] - 100) * 255 / 25500.0, 1.0, 0, 0.5);
									break;
								case 3:
									cv = vec4f( 0, 1.0, (float)(3250 - PressureData[i][j] - 200) * 255 / 25500.0,0.5);
									break;
								case 4:
									cv = vec4f( 0, 1.0 - (float)(3250 - PressureData[i][j] - 300) * 255 / 25500, 1.0,0.5);
									break;
								}
								vertexCol[j * sizex + i] = cv;
							}

						}



						for (int i =0; i< sizex*sizey; i++)
						{
							_mesh->setVertex(i, P3_C4(vertexPos[i], vertexCol[i]));
						}

						delete []vertexPos;
						delete []vertexCol;
			}
			break;
		default:
			return;
		}
	}

	void FengBaoChao::updatedata()
	{
		switch(_curType)
		{
		case Speed:
			{
				static int num =0;
				num = num + 1;
				if (num > 48)
				{
					num = 0;
				}
				//SDL_LockMutex(_loaderMutex);//预读5个
				for (int i =0 ;i <5; i++)
				{
					int val = num +i;
					if(val > 48)
						val-=48;
					if(_dataPool.find(val) == _dataPool.end())
						_dataPool[val] = NULL;
				}
				//SDL_UnlockMutex(_loaderMutex);

				map<int, double*>::iterator iter = _dataPool.find(num);
				if (iter->second)
				{
					int idx =0;
					for (int y = 0; y < numy; y++)
					{
						for (int x = 0; x < numx; x++)
						{
							vel[x][y][0] = iter->second[idx];
							vel[x][y][1] = iter->second[numx*numy+idx++];
						}
					}
					delete [] iter->second;
					_dataPool.erase(iter);
				}
				else 
				{
					num--;
					iframe--;
					return;
				}

				//此处是为了确定最大速度值w，以便进行颜色映射
				//int w = 606;
				//float sss, kkk;
				//sss = 0f; kkk = 0f;

				//HSV模型颜色映射之方向、速度大小
				for (int i = 0; i < numx; i++)
				{
					for (int j = 0; j < numy; j++)
					{
						HSV[i][j][0] = (float)(atan2(vel[i][j][1], vel[i][j][0]) * 180.0 / M_PI);
						if (HSV[i][j][0] < 0) HSV[i][j][0] = HSV[i][j][0] + 360.0;
						//double sk1,sk2;
						//sk1=vel[i, j, 1]+0.0;sk2=vel[i, j, 2]+0.0;
						//HSV[i, j, 0] =(float) Math.Atan2(sk2, sk1);
						int s1 = (int)(vel[i][j][1] * vel[i][j][1] + vel[i][j][0] * vel[i][j][0]);
						HSV[i][j][1] = (1.0f - (float)sqrt(s1 / 606.0f));
						//HSV[i, j, 1] = (float)Math.Sqrt(s1 / 977.0f);
						//if (sss > HSV[i, j, 0]) sss = HSV[i, j, 0];
						//if (kkk < HSV[i, j, 0]) kkk = HSV[i, j, 0];
					}
				}
			}
			break;
		case Temp:
			{
				return;
			}
		case Press:
			{
				static int num =6;
				num = num + 6;
				if (num > 72)
				{
					num = 0;
				}
				//SDL_LockMutex(_loaderMutex);//预读3个
				for (int i =0 ;i <3; i++)
				{
					int val = num +i*6;
					if(val > 72)
						val-=72;
					if(_dataPool.find(val) == _dataPool.end())
						_dataPool[val] = NULL;
				}
				//SDL_UnlockMutex(_loaderMutex);
				memcpy(PressureData1, PressureData2, sizeof(PressureData2));
				map<int, double*>::iterator iter = _dataPool.find(num);
				if (iter->second)
				{
					memcpy(PressureData2, iter->second,sizeof(PressureData2));
					delete [] iter->second;
					_dataPool.erase(iter);
				}
				else 
				{
					num-=6;
					iframe--;
					return;
				}
			}
		default:
			return;
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
				int RandKey = rand()%255;
				phase[i][ j] = RandKey;
			}
		}
		for (k = 0; k < 32; k++)
		{
			t = k * 256 / 32;
			for (i = 0; i < numx; i++)
				for (j = 0; j < numy; j++)
				{
					pat[i][j][k] = lut[(t + phase[i][ j]) % 255];
				}
		}
	}

	int FengBaoChao::workThread()
	{
        cout << "error(call FBC work thread)" << endl;
		/*int toLoad[5];
		char pathName[512];
		char pathName1[512];
		map<int, double*>::iterator iter;
		while (_loader_thread)
		{
			bool	toLoad = false;
			SDL_LockMutex(_loaderMutex);
			iter = _dataPool.begin();
			while(iter != _dataPool.end())
			{
				if(iter->second != NULL)
					iter++;
				else
				{ 
					toLoad = true;
					break;
				}
			}
			SDL_UnlockMutex(_loaderMutex);
			if(toLoad)
			{
				double* data;
				switch(_curType)
				{
				case Speed:
					{
						sprintf(pathName, "VG/taifeng/newVelocity/U10-%d.dat",iter->first);
						sprintf(pathName1, "VG/taifeng/newVelocity/V10-%d.dat",iter->first);
						data = new double[numx* numy*2];

						if (!loadFile(pathName,data)) 
						{ QMessageBox::information(NULL,QStringLiteral("错误"),
						QStringLiteral("数据读取错误"));}

						if (!loadFile(pathName1,data+ numx*numy))
						{ QMessageBox::information(NULL,
						QStringLiteral("错误"),
						QStringLiteral("数据读取错误"));}
					}
					break;
				case Press:
					{
						sprintf(pathName, "VG/taifeng/pressure/700-%d.dat",iter->first);
						data = new double[sizex* sizey];

						if (!loadFile(pathName,data)) 
						{ QMessageBox::information(NULL,QStringLiteral("错误"),
						QStringLiteral("数据读取错误"));}
					}
					break;
				default:
					break;
				}

				SDL_LockMutex(_loaderMutex);
				iter->second = data;
				SDL_UnlockMutex(_loaderMutex);
			}

			if (toLoad == false)
			{
				SDL_Delay(10);
			}
		}*/
		return 1;
	}
