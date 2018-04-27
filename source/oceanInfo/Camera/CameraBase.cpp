#include "CameraBase.h"
#include "../math/mat4.h"
#include "GL/glew.h"
#include <iostream>
#pragma comment(lib, "glu32.lib")

//#include "SDL.h"
//#include "ork/render/FrameBuffer.h"
//ptr<FrameBuffer> fb;
namespace VirtualGlobeScene
{
	// 构造函数初始化摄像机相关信息
	CameraBase::CameraBase(Ellipsoid* ellipsoid) : 
		min_tilt_(radians(0.0)), max_tilt_(radians(85.0)), camera_up_vector(vec3d(0,0,1)),
		rel_camera_pos_(vec3d(0,0,0)), kDoubleTapDelay(3000), 
		fovx_(M_PI /3.0), earth_(ellipsoid),cur_camera_elevation_(0),target_camera_elevation_(0),
		tilt_(0),b_trackball_(true),b_is_auto(true) 
	{
		world_radius_ = ellipsoid->_radii.z;
		distance_ = 2*world_radius_;
		altitude_ = distance_;
		position_ = vec3d(distance_ + world_radius_,0,0);
		maximum_altitude_ = 20 * world_radius_;
		m_orientation_.EulerToQuaternion(radians(105.0f),radians(35.0f),0);
		bank_ = 0;
		terrain_elevation_ =0;
		cur_camera_elevation_ = 0;
		longitude_ = radians(105.0f);
		latitude_ = radians(35.0f);
		heading_ = bank_ =0;
		//fb = FrameBuffer::getDefault();
		sundir_ = vec3d(0,-1,0);
		underground_ = false;
		height_above_terrian_ = altitude_;
		csys_ = CoordinateSystem::GLOBAL;
		local_cenlon_ = local_cenlat_ = 0.0f;
		csys_change_height_ = 10000.0f;
		scr = FULL;
	}

	// 摄像机类析构函数
	CameraBase::~CameraBase()
	{

	}

	void CameraBase::disableTrackBall()
	{
		b_trackball_ = false;
	}
	void CameraBase::enableTrackBall()
	{
		b_trackball_ = true;
	}

	void CameraBase::computeTiltAndHeading()
	{
		if(csys_ == LOCAL)	return;
		if(!b_trackball_)
		{
			// heading
			vec3d up = position_.normalize();
			double dis = position_.length();
			Geodetic3D pos = earth_->ToGeodetic3D(position_);
			vec3d north;
			if ( fabs(pos._latitude) < 1e-3 )
				north = vec3d( 0, 0, 1 );
			else
			{
				north = vec3d( 0, 0, dis / sin(pos._latitude));
				north -= position_;
				if( pos._latitude < 0 )
					north = -north;
				north = north.normalize();
			}
			vec3d east = north.crossProduct( up );
			vec3d eyeToTarget = target_ - position_;
			eyeToTarget = eyeToTarget.normalize();
			double dn,de;
			dn = eyeToTarget.dotproduct( north );
			de = eyeToTarget.dotproduct( east );
			if ( dn && de )
			{
				if( dn > 0 )
					heading_ = atan( de / dn );
				else
				{
					if( de > 0 )
						heading_ = M_PI + atan( de / dn );
					else
						heading_ = -M_PI + atan( de / dn );
				}
			}
			else
			{
				if( dn == 0 )
				{
					heading_ = de * M_PI / ( 2 * fabs(de) );
				}
				else
				{
					if( dn > 0 )
						heading_ = 0;
					else
						heading_ = M_PI;
				}
			}

			// tilt
			// 先求视线与地球的交点
			vec3d p1,p2;
			p1 = eyeToTarget.normalize();
			p2 = position_.normalize();
			p2 = -p2;
			double costheta = p1.dotproduct( p2 );
			if( costheta < 0 )
				tilt_ = max_tilt_;
			else
			{
				double dis = position_.length() * sqrt( 1-costheta*costheta );  // 球心到视线所在直线的距离			
				if( dis >= world_radius_ ) // 不相交
					tilt_ = max_tilt_;
				else
				{
					if( dis < 1e-5 )
						tilt_ = min_tilt_;
					else
					{
						double len = sqrt(position_.length()*position_.length()-dis*dis) // 视点到球面的距离
							-sqrt( world_radius_*world_radius_ - dis*dis);
						vec3d p = position_ + p1*len; // p是视线与球交点位置
						p = p.normalize();
						p1 = -p1;
						tilt_ = acos( p.dotproduct(p1) );
					}
				}
			}
		}		
	}	

	void CameraBase::setCsys(CoordinateSystem mode)
	{
		csys_ = mode;
	}

	void CameraBase::setUp(const vec3d& up)
	{
		if(b_trackball_ || csys_ == CoordinateSystem::LOCAL)
			return;
		if(!b_trackball_)
			up_ = up;
		else
			return;
	}

	// tilt操作需更新altitudet值
	void  CameraBase::set_tilt(double t)
	{
		if(t > max_tilt_)
			t = max_tilt_;
		else if(t < min_tilt_)
			t = min_tilt_;
		tilt_ = t;
		computeAltitude();
	}

	// 根据_distance和_altitude值计算_tilt
	void CameraBase::computeTilt()
	{
		if(csys_ == CoordinateSystem::GLOBAL && b_trackball_)
		{
			double a = world_radius_+altitude_;
			double b = distance_;
			double c = world_radius_;
			tilt_ = acos((a*a+b*b-c*c)/(2*a*b));
		}		
	}

	double CameraBase::tilt()
	{
		return tilt_;
	}

	double CameraBase::altitudeAboveTerrain()
	{
		return altitude_ - terrain_elevation_;
	}

	// distance_赋值后，需重新计算altitude_
	void CameraBase::set_distance(double val)
	{
		distance_ = val;
		computeAltitude();
	}

	double CameraBase::distance()
	{
		return distance_;
	}

	// 计算_distance,为保持交互一致性，地球半径取实际半径，即地球模型半径+地形高度
	void CameraBase::computeDistance()
	{
		if(csys_ == CoordinateSystem::GLOBAL && b_trackball_)
		{
			double ang = M_PI-tilt_;
			double cos = std::cos(ang);
			double actualRaduis = world_radius_ + terrain_elevation_;
			double x = actualRaduis*cos;
			double hyp = world_radius_+altitude_;
			double y = sqrt(actualRaduis*actualRaduis*cos*cos+hyp*hyp-actualRaduis*actualRaduis);
			double res = x-y;
			if(res<0)
				res = x+y;
			distance_ = res;
		}		
	}

	// 计算视点高度值，同样取地球半径为实际半径
	bool CameraBase::computeAltitude()
	{
		if(csys_ == CoordinateSystem::GLOBAL && b_trackball_)
		{
			double actualRaduis = world_radius_ + terrain_elevation_;
			double dfromeq = sqrt(actualRaduis*actualRaduis + distance_*distance_ - 
				2 * actualRaduis*distance_*cos(M_PI-tilt_));
			double alt = dfromeq - world_radius_;

			if(alt > maximum_altitude_)
				alt = maximum_altitude_;
			altitude_ = alt;

			return true;
		}
		return false;		
	}

	void CameraBase::computeAbsoluteMatrices()
	{
		float aspectRatio =  (float)viewport_.z/ viewport_.w;
		if (b_is_auto)
		{
			float zNear = (float)abs(altitude_)*0.005f;
			if(zNear <2.5)
				zNear =0.5;
			else if (zNear <5)
			{
				zNear = 1.5;
			}
			else if(zNear<15)
				zNear = 5;
			perspective_near_plane_distance_ = zNear;
			if(altitude_ >=0 )
			{
				if(altitude_ <= 65000)
				{
					double distToCenterOfPlanet = (altitude_ + world_radius_);
					double tangentalDistance  = sqrt(distToCenterOfPlanet*distToCenterOfPlanet 
						- world_radius_*world_radius_);
					//这个是切线求法，但是有地球外物体特别是卫星轨迹时不合理
					perspective_far_plane_distance_ = tangentalDistance;
				}
				else
					perspective_far_plane_distance_ = altitude_ + 2.4*world_radius_;
				if(perspective_far_plane_distance_ < 1000000)
					perspective_far_plane_distance_ = 1000000;
			}
			else
				perspective_far_plane_distance_ = 30000 + fabs(altitude_)*10;
		}
		
		float vfov;
		//if(aspectRatio <2.5)//单屏
		//{
			vfov= degrees(atan(tan(fovx_/2.0)/(float)viewport_.z
					*viewport_.w))*2.0;
		//}
		//else
		//{
		//	vfov= degrees(atan(tan(fovx_)/(float)viewport_.z
		//			*viewport_.w))*2.0;
		//}
		fovy_ = radians(vfov);
		aspect_ = aspectRatio;
		if(scr == FULL)
		{
		m_absolute_projection_matrix_ = mat4f::perspectiveProjection(vfov, 
			(float)viewport_.z / viewport_.w,
			perspective_near_plane_distance_,(float)perspective_far_plane_distance_);
		}
		else
		{
			float focus_ = 3;
			float screenWidth = 10;
			int winWidth = viewport().z*3;
			int winHeight = viewport().w;
			float screenHeight = screenWidth /(float)winWidth*(float)winHeight;
			float left,right,botom,top;
			float nr = perspective_near_plane_distance();
			nr = nr/focus_;
			float len = nr*screenWidth;
			botom = (-screenHeight/2.0)*nr;
			top = (screenHeight/2.0)*nr;
			if(scr == LEFT)
				left = -len/2;
			else if(scr == MIDDLE)
				left = -len/2 + len/3;
			else
				left = -len/2 + 2.0*len/3;
			right = left + len/3;	
			m_absolute_projection_matrix_ = mat4f::frustum(left,right,botom,top,
				perspective_near_plane_distance(),perspective_far_plane_distance());
		}

		vec3d pos = earth_->ToVector3D(Geodetic3D(longitude_, latitude_, 0));
		pos = pos.normalize() * (world_radius_ + cur_camera_elevation_);
		m_absolute_view_matrix_ = mat4f::lookAt(vec3f(pos.x, pos.y,pos.z), 
			vec3f::ZERO, vec3f(0,0,1)).transpose();
		mat4f rypMatrix = mat4f::RotationYawPitchRoll(0, -tilt_,heading_);
		//rypMatrix = rypMatrix.transpose();
		m_absolute_view_matrix_ = m_absolute_view_matrix_ * rypMatrix;
		m_absolute_view_matrix_ = m_absolute_view_matrix_ 
			* mat4f::translate(vec3f(0,0,-distance_ - 0)).transpose();
		m_absolute_view_matrix_ = m_absolute_view_matrix_ 
			* mat4f::rotatez(degrees(bank_)).transpose();
		m_absolute_view_matrix_ = m_absolute_view_matrix_.transpose();
	}

	void CameraBase::computeViewMatrix() 
	{
		computeAbsoluteMatrices();
		int minStep = 0.3;//这个尺度值要调节，近地面不能太大，否则会抖动，特别是建筑物
		double hei = altitudeAboveTerrain();
		if (hei>10000)
		{
			minStep = 10;
		}
		else if (hei>1000)
		{
			minStep = 1;
		}
		else if(hei>400)
			minStep = 0.5;
		if( !underground_ )
			target_camera_elevation_ = terrain_elevation_ * 1.0;
		else if( altitude_ < terrain_elevation_ )
			target_camera_elevation_ = altitude_ * 1.0;
		else
			target_camera_elevation_ = terrain_elevation_;

		float stepToTarget = target_camera_elevation_ - cur_camera_elevation_;
		if(abs(stepToTarget) > minStep)
		{
			float step = 0.05f * stepToTarget;
			if (abs(step) < minStep) step = step > 0 ? minStep : -minStep;
			cur_camera_elevation_ = cur_camera_elevation_ + step;
		}
		else cur_camera_elevation_ = target_camera_elevation_;

		// needs to be double precsion
		double radius = world_radius_ + cur_camera_elevation_;
		double radCosLat = radius*cos(latitude_);
		vec3d LookFrom(radCosLat * cos(longitude_),
			radCosLat * sin(longitude_),
			radius * sin(latitude_));
		//relCameraPos = LookFrom;
		// this constitutes a local tri-frame hovering above the sphere		
		vec3d zAxis = LookFrom.normalize(); // on sphere the normal vector and position vector are the same
		vec3d xAxis = camera_up_vector.crossProduct(zAxis).normalize();
		vec3d yAxis = zAxis.crossProduct(xAxis);
		Geodetic3D p((float)longitude_, (float)latitude_,0);
		reference_center_ = earth_->ToVector3D(p);
		vec3d relCameraPos = LookFrom - reference_center_;
		m_view_matrix_ = mat4f((float)xAxis.x, (float)yAxis.x, (float)zAxis.x, 0,
							(float)xAxis.y,(float)yAxis.y, (float)zAxis.y,0,
							(float)xAxis.z, (float)yAxis.z, (float)zAxis.z,0,
							-(float)(xAxis.x * relCameraPos.x + xAxis.y * relCameraPos.y + xAxis.z * relCameraPos.z),
							-(float)(yAxis.x * relCameraPos.x + yAxis.y * relCameraPos.y + yAxis.z * relCameraPos.z),
							-(float)(zAxis.x * relCameraPos.x + zAxis.y * relCameraPos.y + zAxis.z * relCameraPos.z),1);
		double cameraDisplacement = distance_;
		if(cameraDisplacement < 0.01)
			cameraDisplacement = 0.05;

		mat4f rypMatrix = mat4f::RotationYawPitchRoll(0, -tilt_,heading_);
		m_view_matrix_ = m_view_matrix_* rypMatrix;
		m_view_matrix_ = m_view_matrix_*mat4f::translate(vec3f(0,0,
			-cameraDisplacement + 0)).transpose();
		m_view_matrix_ = m_view_matrix_* mat4f::rotatez(
			degrees(bank_)).transpose();
		m_view_matrix_ = m_view_matrix_.transpose();
		position_ = (m_view_matrix_.inverse()* vec4f(0,0,0,1.0)).xyz().cast<double>() 
			+ reference_center_;
	}

	mat4f CameraBase::m_view_matrix()
	{
		return m_view_matrix_;
	}

	mat4d CameraBase::m_world_matrix()
	{
		return m_world_matrix_;
	}

	mat4f CameraBase::m_absolute_projection_matrix()
	{
		return m_absolute_projection_matrix_;
	}

	mat4f CameraBase::m_absolute_view_matrix()
	{
		return m_absolute_view_matrix_;
	}

	vec3d CameraBase::reference_center()
	{
		return reference_center_;
	}

	vec3d CameraBase::position()
	{
		return position_;
	}

	double CameraBase::heading()
	{
		return heading_;
	}

	double CameraBase::altitude()
	{
		return altitude_;
	}

	double CameraBase::world_radius()
	{
		return world_radius_;
	}

	bool CameraBase::b_trackball()
	{
		return b_trackball_;
	}

	vec3d CameraBase::up()
	{
		return up_;
	}

	vec3d CameraBase::target()
	{
		return target_;
	}

	vec3d CameraBase::sundir()
	{
		return sundir_;
	}

	void CameraBase::set_sundir(const vec3d& sundir)
	{
		sundir_ = sundir;
	}

	CoordinateSystem CameraBase::csys()
	{
		return csys_;
	}

	mat4d CameraBase::local_world2local()
	{
		if(csys_ == CoordinateSystem::LOCAL)
		{
			return local_world2local_;
		}
		else
		{
			mat4d tmp;
			return tmp;
		}
	}

	mat4d CameraBase::local_local2world()
	{
		if(csys_ == CoordinateSystem::LOCAL)
		{
			return local_local2world_;
		}
		else
		{
			mat4d tmp;
			return tmp;
		}
	}

	float CameraBase::perspective_far_plane_distance()
	{
		return perspective_far_plane_distance_;
	}

	float CameraBase::perspective_near_plane_distance()
	{
		return perspective_near_plane_distance_;
	}

	double CameraBase::longitude()
	{
		return longitude_;
	}

	double CameraBase::latitude()
	{
		return latitude_;
	}

	double CameraBase::fovx()
	{
		return fovx_;
	}
	void CameraBase::setFovx(double fov)
	{
		fovx_ = fov;
	}
	double CameraBase::fovy()
	{
		return fovy_;
	}

	double CameraBase::aspect()
	{
		return aspect_;
	}

	vec3d CameraBase::cureast()
	{
		return cureast_;
	}

	vec3d CameraBase::curnorth()
	{
		return curnorth_;
	}

	vec3d CameraBase::curup()
	{
		return curup_;
	}

	float CameraBase::csys_change_height()
	{
		return csys_change_height_;
	}

	short CameraBase::terrain_elevation()
	{
		return terrain_elevation_;
	}

	double CameraBase::height_above_terrian()
	{
		return height_above_terrian_;
	}

	vec4i CameraBase::viewport()
	{
		return viewport_;
	}

	void CameraBase::set_viewport(const vec4i &viewport)
	{
		viewport_ = viewport;
	}

	vec3d CameraBase::unproject(const vec3d& v)
	{
        //std::cout << "Fatal Error: Funtion CameraBase::unproject is not implemented!" << std::endl;

		mat4d worldToCamera = m_view_matrix_.cast<double>().transpose();
		mat4d cameraToScreen = m_absolute_projection_matrix_.cast<double>().transpose();
		GLint vp[4];
		vp[0] = 0;
		vp[1] =0;
		vp[2] = viewport_.z;
		vp[3] = viewport_.w;
		double hitPoint[3];
		gluUnProject(v.x,v.y,v.z,worldToCamera.coefficients(),
					cameraToScreen.coefficients(),vp,&hitPoint[0],
					&hitPoint[1],&hitPoint[2]);
		vec3d worldPos(hitPoint[0], hitPoint[1], hitPoint[2]);
		/*vec4d posscreen = vec4d(v.x / viewport_.z * 2 -1, v.y / viewport_.w * 2 -1, v.z, 1.0);
		vec4d posworld = m_view_matrix_.cast<double>().inverse() * m_absolute_projection_matrix_.cast<double>().inverse() * posscreen;
		posworld /= posworld.w;
		vec3d worldPos = posworld.xyz();*/
		if(csys_ == CoordinateSystem::GLOBAL)
		{
			worldPos += reference_center_;
		}
		else
		{
			worldPos = local_local2world_*worldPos;
		}
		return worldPos;
 	}

	vec3d CameraBase::getDir()
	{
		if(csys_ == GLOBAL && !b_trackball_)
		{
			return (target_ - position_).normalize();
		}
		/*if(csys_ == CoordinateSystem::LOCAL)
		{
			vec3d res = local_local2world_ * local_dir_;
			return res.normalize();
		}*/
		vec4f pp= m_absolute_view_matrix_.inverse() * vec4f(0,0,-10000,1.0);
		vec4d p(pp.x, pp.y, pp.z, 1.0);
		if(csys_ == LOCAL)
			p = local_local2world_ * p;
		vec3d res(p.x, p.y, p.z);
		if(csys_ == GLOBAL && tilt_ < 1e-4)
			res = vec3d(0,0,0);
		return (res - position_).normalize();
	}

	// 根据屏幕坐标反求经纬度，仅用于GLOBAL模式下
	bool CameraBase::pickingRayIntersection(int screenx, int screeny, 
											double &latitude, double &longitude) // screenxy为整个窗口以左下角为0，0的坐标
	{

		//std::cout << "Unfinished" << std::endl;
		vec3d point;
		//fb->setReadBuffer(DEPTH);
		float data = 1.0f;
		glReadPixels(screenx, screeny, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &data);

		//fb->readPixels(screenx,screeny,1,1,
		//				DEPTH_COMPONENT,PixelType::FLOAT,Buffer::Parameters(),
		//				CPUBuffer(&data));
		if (data > 0.999999f)
		{
			latitude = 0;
			longitude = 0;
			return false;
		}
		vec3d v1(screenx-viewport_.x, screeny-viewport_.y, 0);
		vec3d p1 = unproject(v1);

		vec3d v2(screenx-viewport_.x, screeny-viewport_.y, 1.0);
		vec3d p2 = unproject(v2);
		double a = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y) 
			+ (p2.z - p1.z) * (p2.z - p1.z);
		double b = 2.0*((p2.x - p1.x)*(p1.x) + (p2.y - p1.y)*(p1.y) 
			+ (p2.z - p1.z)*(p1.z));
		double c = p1.x*p1.x + p1.y*p1.y + p1.z*p1.z - world_radius_ * world_radius_;

		double discriminant = b*b - 4 * a * c;
        double t1 = ((-1.0) * b - sqrt(b*b - 4 * a * c)) / (2*a);
		if(discriminant <= 0 || altitude_<0 || t1<=0 )//t1是一种异常情况，正常做下去会视点后退计算，暂时不知道里面的计算原理
		{
			//与地球模型求交不成功，改为直接求位置模式
			point = unproject(vec3d(screenx-viewport_.x,screeny-viewport_.y,data));
			Geodetic3D i1t = earth_->ToGeodetic3D(point);
			latitude = i1t.getLatitude();
			longitude = i1t.getLongitude();
			return false;
		}
		vec3d i1(p1.x + t1*(p2.x - p1.x), p1.y + t1*(p2.y - p1.y), p1.z 
			+ t1 *(p2.z - p1.z));
		Geodetic3D i1t = earth_->ToGeodetic3D(vec3d(i1.x, i1.y, i1.z));
		latitude = i1t.getLatitude();
		longitude = i1t.getLongitude();
		return true;
	}

	/*void CameraBase::pointGoto(double lat, double lon )
	{
		if(csys_ == CoordinateSystem::GLOBAL && b_trackball_)
		{
			setPosition(lat, lon);
		}
		else return;
	}*/

	void CameraBase::setPosition(double lat, double lon)
	{
		//继承类实现掉
	}

	
	void CameraBase::updateNonTrackBall()
	{
		float aspectRatio =  (float)viewport_.z/ viewport_.w;
		if (b_is_auto)
		{
			float zNear;
			Geodetic3D geoPos = earth_->ToGeodetic3D(position_);
			altitude_ = geoPos.getHeight();
			if(altitude_ > 50000)//这个模式还这么高可能是卫星
				zNear = 100;
			else
			{
				zNear = (float)abs(altitude_)*0.005f;

				if(zNear <2.5)
					zNear =0.05;
				else if (zNear <5)
				{
					zNear = 0.15;
				}
				else if(zNear<15)
					zNear = 0.5;
			}

			perspective_near_plane_distance_ = zNear;

			if(altitude_ >=0)
			{
				if(altitude_ <= 65000)
				{
					double distToCenterOfPlanet = (altitude_ + world_radius_);
					double tangentalDistance  = sqrt(distToCenterOfPlanet*distToCenterOfPlanet 
						- world_radius_*world_radius_);
					perspective_far_plane_distance_ = tangentalDistance;//这个是切线求法，但是有地球外物体特别是卫星轨迹时不合理
				}
				else
				{
					perspective_far_plane_distance_ = altitude_ + 1.8*world_radius_;
				}
				if(perspective_far_plane_distance_ < 500000)
					perspective_far_plane_distance_ = 500000;
			}
			else
				perspective_far_plane_distance_ = 100000;
		}
		
		float vfov;
		//if(aspectRatio <2.5)//单屏
			vfov= degrees(atan(tan(fovx_/2.0)/(float)viewport_.z
			*viewport_.w))*2.0;
		/*else
			vfov= degrees(atan(tan(fovx_)/(float)viewport_.z
			*viewport_.w))*2.0;
		*/if(scr == FULL)
		{
		m_absolute_projection_matrix_ = mat4f::perspectiveProjection(vfov, 
			(float)viewport_.z / viewport_.w,
			perspective_near_plane_distance_,(float)perspective_far_plane_distance_);
		}
		else
		{
			float focus_ = 3;
			float screenWidth = 10;
			int winWidth = viewport().z*3;
			int winHeight = viewport().w;
			float screenHeight = screenWidth /(float)winWidth*(float)winHeight;
			float left,right,botom,top;
			float nr = perspective_near_plane_distance();
			nr = nr/focus_;
			float len = nr*screenWidth;
			botom = (-screenHeight/2.0)*nr;
			top = (screenHeight/2.0)*nr;
			if(scr == LEFT)
				left = -len/2;
			else if(scr == MIDDLE)
				left = -len/2 + len/3;
			else
				left = -len/2 + 2.0*len/3;
			right = left + len/3;	
			m_absolute_projection_matrix_ = mat4f::frustum(left,right,botom,top,
				perspective_near_plane_distance(),perspective_far_plane_distance());
		}

		m_view_matrix_ = mat4f::lookAt(vec3f(0,0,0), 
			(target_-position_).cast<float>(),up_.cast<float>());
		m_absolute_view_matrix_ = mat4f::lookAt(position_.cast<float>(), 
			target_.cast<float>(),up_.cast<float>());
		reference_center_ = position_;

		computeTiltAndHeading();
	}

	void CameraBase::updateTrackBall()
	{
		vec3d p = m_orientation_.QuaternionToEuler();
		latitude_= p.y;
		longitude_ = p.x;
		heading_ = p.z;
		static double lat,log;

		
		terrain_elevation_ =0;
		cur_terrain_elevation_ = 0;
		lat = latitude_;
		log = longitude_;
		height_above_terrian_ = altitude_ - terrain_elevation_;
		computeViewMatrix();
	}

	void CameraBase::updateLocal()
	{
		float aspectRatio =  (float)viewport_.z/ viewport_.w;
		if (b_is_auto)
		{
			float zNear;

			position_ = local_local2world_*local_eye_;
			Geodetic3D geoPos = earth_->ToGeodetic3D(position_);
			altitude_ = geoPos.getHeight();

			zNear = (float)abs(altitude_)*0.01f; ////徐以波为解决水面闪烁临时调整near和高度的关系 原来是0.005f

			if(zNear <2.5)
				zNear =0.05;
			else if (zNear <5)
			{
				zNear = 0.15;
			}
			else if(zNear<15)
				zNear = 0.5;		

			if (zNear < 1.0)      //徐以波为解决水面闪烁临时限制near最小为5.0
			{
				zNear = 1.0;
			}

			perspective_near_plane_distance_ = zNear;

			if(altitude_ >=0)
			{				
				double distToCenterOfPlanet = (altitude_ + world_radius_);
				double tangentalDistance  = sqrt( distToCenterOfPlanet*distToCenterOfPlanet 
					- world_radius_*world_radius_);
				perspective_far_plane_distance_ = tangentalDistance;//这个是切线求法，但是有地球外物体特别是卫星轨迹时不合理

				if(perspective_far_plane_distance_ < 500000)
					perspective_far_plane_distance_ = 500000;
			}
			else
				perspective_far_plane_distance_ = 100000;
		}
		
		float vfov;
		//if(aspectRatio <2.5)//单屏
		//{
			vfov= degrees(atan(
			tan(fovx_/2.0)/(float)viewport_.z*viewport_.w))*2.0;
		/*}
		else
		{
			vfov= degrees(
				atan(tan(fovx_)/(float)viewport_.z*viewport_.w))*2.0;
		}*/
		if(scr == FULL)
		{
		m_absolute_projection_matrix_ = mat4f::perspectiveProjection(vfov, 
			(float)viewport_.z / viewport_.w,
			perspective_near_plane_distance_,(float)perspective_far_plane_distance_);		
		}
		else
		{
			float focus_ = 3;
			float screenWidth = 10;
			int winWidth = viewport().z*3;
			int winHeight = viewport().w;
			float screenHeight = screenWidth /(float)winWidth*(float)winHeight;
			float left,right,botom,top;
			float nr = perspective_near_plane_distance();
			nr = nr/focus_;
			float len = nr*screenWidth;
			botom = (-screenHeight/2.0)*nr;
			top = (screenHeight/2.0)*nr;
			if(scr == LEFT)
				left = -len/2;
			else if(scr == MIDDLE)
				left = -len/2 + len/3;
			else
				left = -len/2 + 2.0*len/3;
			right = left + len/3;	
			m_absolute_projection_matrix_ = mat4f::frustum(left,right,botom,top,
				perspective_near_plane_distance(),perspective_far_plane_distance());
		}

		m_view_matrix_ = mat4f::lookAt(vec3f(0,0,0), (local_dir_).cast<float>(),
			local_up_.cast<float>());
		m_absolute_view_matrix_ = mat4f::lookAt(local_eye_.cast<float>(), 
			local_target_.cast<float>(),local_up_.cast<float>());
		reference_center_ = local_origin_;
		m_view_matrix_ = m_absolute_view_matrix_;		
	}

	void CameraBase::update()
	{
		if(csys_ == CoordinateSystem::GLOBAL)
		{
			if(b_trackball_)
				updateTrackBall();	
			else
				updateNonTrackBall();
		}
		else if(csys_ == CoordinateSystem::LOCAL)
		{
			updateLocal();
		}
		mat4f rtemdl = m_view_matrix_.transpose();
		mat4f prj = m_absolute_projection_matrix_.transpose();
		float m[16], n[16];
		memcpy(m, rtemdl.coefficients(),16*sizeof(float));
		memcpy(n, prj.coefficients(),16*sizeof(float));
		getCurDir(curup_,curnorth_,cureast_);
		local2world_ = getLocalToWorldTransform();
		updateFrustum(m, n);
	}

	void CameraBase::updateFrustum(float* modl,float* proj)
	{
		float clip[16];
		float t; /* Get the current PROJECTION matrix from OpenGL */
		clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] 
					+ modl[ 2] * proj[ 8] +    modl[ 3] * proj[12];
		clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] 
					+ modl[ 2] * proj[ 9] +    modl[ 3] * proj[13];
		clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] 
					+ modl[ 2] * proj[10] +    modl[ 3] * proj[14];
		clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] 
					+ modl[ 2] * proj[11] +    modl[ 3] * proj[15];  
		clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4]    
					+ modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
		clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] 
					+ modl[ 6] * proj[ 9] +    modl[ 7] * proj[13];
		clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] 
					+ modl[ 6] * proj[10] +    modl[ 7] * proj[14];
		clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] 
					+ modl[ 6] * proj[11] +    modl[ 7] * proj[15];   clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4]    + modl[10] * proj[ 8] + modl[11] * proj[12];
		clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] 
					+ modl[10] * proj[ 9] +    modl[11] * proj[13];
		clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] 
					+ modl[10] * proj[10] +    modl[11] * proj[14];
		clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] 
					+ modl[10] * proj[11] +    modl[11] * proj[15];   clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4]    + modl[14] * proj[ 8] + modl[15] * proj[12];
		clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] 
					+ modl[14] * proj[ 9] +    modl[15] * proj[13];
		clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] 
					+ modl[14] * proj[10] +    modl[15] * proj[14];
		clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] 
					+ modl[14] * proj[11] +    modl[15] * proj[15]; /* Extract the numbers for the RIGHT plane */
		frustum_val[0][0] = clip[ 3] - clip[ 0];
		frustum_val[0][1] = clip[ 7] - clip[ 4];
		frustum_val[0][2] = clip[11] - clip[ 8];
		frustum_val[0][3] = clip[15] - clip[12]; /* Normalize the result */
		t = sqrt( frustum_val[0][0] * frustum_val[0][0] 
				+ frustum_val[0][1] * frustum_val[0][1] 
				+ frustum_val[0][2] * frustum_val[0][2] );
		frustum_val[0][0] /= t;
		frustum_val[0][1] /= t;
		frustum_val[0][2] /= t;
		frustum_val[0][3] /= t; /* Extract the numbers for the LEFT plane */
		frustum_val[1][0] = clip[ 3] + clip[ 0];
		frustum_val[1][1] = clip[ 7] + clip[ 4];
		frustum_val[1][2] = clip[11] + clip[ 8];
		frustum_val[1][3] = clip[15] + clip[12]; /* Normalize the result */
		t = sqrt( frustum_val[1][0] * frustum_val[1][0] 
				+ frustum_val[1][1] * frustum_val[1][1] 
				+ frustum_val[1][2] * frustum_val[1][2] );
		frustum_val[1][0] /= t;
		frustum_val[1][1] /= t;
		frustum_val[1][2] /= t;
		frustum_val[1][3] /= t; /* Extract the BOTTOM plane */
		frustum_val[2][0] = clip[ 3] + clip[ 1];
		frustum_val[2][1] = clip[ 7] + clip[ 5];
		frustum_val[2][2] = clip[11] + clip[ 9];
		frustum_val[2][3] = clip[15] + clip[13]; /* Normalize the result */
		t = sqrt( frustum_val[2][0] * frustum_val[2][0] 
				+ frustum_val[2][1] * frustum_val[2][1] 
				+ frustum_val[2][2] * frustum_val[2][2] );
		frustum_val[2][0] /= t;
		frustum_val[2][1] /= t;
		frustum_val[2][2] /= t;
		frustum_val[2][3] /= t; /* Extract the TOP plane */
		frustum_val[3][0] = clip[ 3] - clip[ 1];
		frustum_val[3][1] = clip[ 7] - clip[ 5];
		frustum_val[3][2] = clip[11] - clip[ 9];
		frustum_val[3][3] = clip[15] - clip[13]; /* Normalize the result */
		t = sqrt( frustum_val[3][0] * frustum_val[3][0] 
				+ frustum_val[3][1] * frustum_val[3][1] 
				+ frustum_val[3][2] * frustum_val[3][2] );
		frustum_val[3][0] /= t;
		frustum_val[3][1] /= t;
		frustum_val[3][2] /= t;
		frustum_val[3][3] /= t; /* Extract the FAR plane */
		frustum_val[4][0] = clip[ 3] - clip[ 2];
		frustum_val[4][1] = clip[ 7] - clip[ 6];
		frustum_val[4][2] = clip[11] - clip[10];
		frustum_val[4][3] = clip[15] - clip[14]; /* Normalize the result */
		t = sqrt( frustum_val[4][0] * frustum_val[4][0] 
				+ frustum_val[4][1] * frustum_val[4][1] 
				+ frustum_val[4][2] * frustum_val[4][2] );
		frustum_val[4][0] /= t;
		frustum_val[4][1] /= t;
		frustum_val[4][2] /= t;
		frustum_val[4][3] /= t; /* Extract the NEAR plane */
		frustum_val[5][0] = clip[ 3] + clip[ 2];
		frustum_val[5][1] = clip[ 7] + clip[ 6];
		frustum_val[5][2] = clip[11] + clip[10];
		frustum_val[5][3] = clip[15] + clip[14]; /* Normalize the result */
		t = sqrt( frustum_val[5][0] * frustum_val[5][0] 
				+ frustum_val[5][1] * frustum_val[5][1] 
				+ frustum_val[5][2] * frustum_val[5][2] );
		frustum_val[5][0] /= t;
		frustum_val[5][1] /= t;
		frustum_val[5][2] /= t;
		frustum_val[5][3] /= t;
	}

	int CameraBase::cubeInFrustum(vec3f* cube)
	{
		int iTotalIn = 0;
		for(int i = 0; i < 6; ++i)
		{

			int iInCount = 8;
			int iPtIn = 1;	
			for (int j =0; j< 8; j++)
			{
				if(frustum_val[i][0] * cube[j].x + frustum_val[i][1] * cube[j].y 
					+ frustum_val[i][2] * cube[j].z + frustum_val[i][3] < 0)
				{
					iPtIn = 0;
					--iInCount;
				}
			}

			// were all the points outside of plane p?
			if(iInCount == 0)
				return false;		// check if they were all on the right side of the plane
			iTotalIn += iPtIn;
		}	// so if iTotalIn is 6, then all are inside the view
		if(iTotalIn == 6)
			return 1;	// we must be partly in then otherwise
		return 2;
	}

	bool CameraBase::sphereInFrustum(const vec3d &center, float radius)
	{	
		vec3d tmp;
		tmp = center;
		if( csys_ == LOCAL )
			tmp = local_world2local_*center;
		else
			tmp -= reference_center_;

		int p;

		for( p = 0; p < 6; p++ )
			if( frustum_val[p][0] * tmp.x + frustum_val[p][1] * tmp.y + 
				frustum_val[p][2] * tmp.z + frustum_val[p][3]    <= -radius )
				return false;
		return true;
	}

	vec3d CameraBase::getPartCsysDir()
	{
		vec3d dir = getDir();
		vec4f tmp(dir.x, dir.y, dir.z, 0.0);
		tmp = world2local_*tmp;
		dir = vec3d(tmp.x, tmp.y, tmp.z);
		dir = dir.normalize();
		return dir;
	}

	vec3d CameraBase::getPartCsysUp()
	{
		vec3d up = getCameraUpVector();
		vec4f tmp(up.x, up.y, up.z, 0.0);
		tmp = world2local_*tmp;
		up = vec3d(tmp.x, tmp.y, tmp.z);
		up = up.normalize();
		return up;
	}

	vec3d CameraBase::getCameraUpVector()
	{
		if(csys_ == CoordinateSystem::GLOBAL)
		{
			if(!b_trackball_)
			{
				return up_;
			}
			else
			{
				return camera_up_vector;
			}
		}
		else
		{
			vec3d res = local_local2world_*local_up_;
			res = res.normalize();
			return res;
		}		
	}

	Geodetic3D CameraBase::getEyeGeo()
	{
		Geodetic3D res = earth_->ToGeodetic3D(position_);
		return res;
	}

	void CameraBase::getCurDir(vec3d &up, vec3d &north, vec3d &east)
	{
		up  =vec3d( cos(longitude_)*cos(latitude_), 
					sin(longitude_)*cos(latitude_), sin(latitude_));

		// Compute east vector
		east =vec3d(-sin(longitude_), cos(longitude_), 0);
		// Compute north vector = outer product up x east
		north   = up.crossProduct(east);
		north = north.normalize();		 
	}
	 
	// 计算局部坐标到世界坐标的转换矩阵，仅用于GLOBAL模式
	mat4f CameraBase::getLocalToWorldTransform()
	{
		vec3d pos = position_;
		Geodetic3D ll = earth_->ToGeodetic3D(pos);
		mat4f localtoWorld= mat4f::translate(vec3f(pos.x, pos.y, pos.z));
		// Compute up vector
		vec3d up(cos(ll.getLongitude())*cos(ll.getLatitude()), 
					sin(ll.getLongitude())*cos(ll.getLatitude()), 
					sin(ll.getLatitude()));

		// Compute east vector
		vec3d east(-sin(ll.getLongitude()), cos(ll.getLongitude()), 0);
		// Compute north vector = outer product up x east
		vec3d north = up.crossProduct(east);

		// set matrix
		float m[4][4];
		memcpy(m, localtoWorld.coefficients(), sizeof(float)*16);
		m[0][0] = east[0];
		m[1][0] = east[1];
		m[2][0] = east[2];

		m[0][1] = north[0];
		m[1][1] = north[1];
		m[2][1] = north[2];

		m[0][2] = up[0];
		m[1][2] = up[1];
		m[2][2] = up[2];

		localtoWorld = mat4f(m);
		local2world_ = localtoWorld;
		world2local_ = local2world_.inverse();
		return localtoWorld;
	}

	// 计算局部坐标和全局坐标相互转换的矩阵，仅用于LOCAL模式
	void CameraBase::computeLocalWorldTrans()
	{	
		if(csys_ != CoordinateSystem::LOCAL)
		{
			return;
		}
		Geodetic3D ll(longitude_, latitude_, 0);
		vec3d pos = earth_->ToVector3D(ll);
		local_local2world_ = mat4d::translate(pos);
		// Compute up vector
		vec3d up(cos(ll.getLongitude())*cos(ll.getLatitude()), 
					sin(ll.getLongitude())*cos(ll.getLatitude()), 
					sin(ll.getLatitude()));
		// Compute east vector
		vec3d east(-sin(ll.getLongitude()), cos(ll.getLongitude()), 0);
		// Compute north vector = outer product up x east
		vec3d north = up.crossProduct(east);

		// set matrix
		double m[4][4];
		memcpy(m, local_local2world_.coefficients(), sizeof(double)*16);
		m[0][0] = east[0];
		m[1][0] = east[1];
		m[2][0] = east[2];

		m[0][1] = north[0];
		m[1][1] = north[1];
		m[2][1] = north[2];

		m[0][2] = up[0];
		m[1][2] = up[1];
		m[2][2] = up[2];

		local_local2world_ = mat4d(m);		
		local_world2local_ = local_local2world_.inverse();
	}

	bool CameraBase::underground()
	{
		return underground_;
	}

	void CameraBase::set_underground(bool underground)
	{
		underground_ = underground;
	}

	vec3d CameraBase::local_up()
	{
		if(csys_ == CoordinateSystem::LOCAL)
		{
			return local_up_;
		}
		else
		{
			return vec3d(0.0, 0.0, 0.0);
		}
	}

	vec3d CameraBase::local_dir()
	{
		if(csys_ == CoordinateSystem::LOCAL)
		{
			return local_dir_;
		}
		else
		{
			return vec3d(0.0, 0.0, 0.0);
		}		
	}
	
	vec3d CameraBase::local_eye()
	{
		if(csys_ == CoordinateSystem::LOCAL)
		{
			return local_eye_;
		}
		else
		{
			return vec3d(0.0, 0.0, 0.0);
		}
	}

	vec3d CameraBase::local_origin()
	{
		if(csys_ == CoordinateSystem::LOCAL)
		{
			return local_origin_;
		}
		else
		{
			return vec3d(0.0, 0.0, 0.0);
		}
	}
}

