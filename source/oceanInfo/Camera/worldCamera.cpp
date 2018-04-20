#include "worldCamera.h"
//#include "SDL.h"
namespace VirtualGlobeScene
{
	// 构造函数--参数初始化
	WorldCamera::WorldCamera(Ellipsoid *ellipsoid) : 
		CameraBase(ellipsoid)
	{
		target_orientation_ = m_orientation_;
		target_distance_ = distance_;
		target_altitude_ = altitude_;
		target_tilt_ = tilt_;
		target_fov_ = fovx_;
		target_bank_ = 0;
		target_latitude_ = latitude_;
		target_longitude_ = longitude_;
		target_heading_ =0;
		b_disableslerp = false;
		camera_has_momentum_ = false;
		status_ = 0;
	}

	WorldCamera::~WorldCamera()
	{

	}

	// target_tilt_赋值之后重新计算target_altitude_
	void WorldCamera::set_target_tilt(double t)
	{
		if(csys_ == CoordinateSystem::LOCAL)
			return;

		if(t > max_tilt_)
			t = max_tilt_;
		else if(t < min_tilt_)
			t = min_tilt_;
		target_tilt_ = t;
		computeTargetAltitude();
	}

	void WorldCamera::reLocate()
	{
		if(csys_ == CoordinateSystem::LOCAL || !b_trackball_)
			return;
		heading_ = target_heading_ = 0.0f;
		set_target_tilt(0.0);		
		double height = fabs(altitude_)*1.5 + 1000;
		if(height > world_radius_)
			height = world_radius_;
		setPosition(latitude_, longitude_, height, 0);		
	}

	void WorldCamera::resetHead()
	{
		heading_ = target_heading_ = 0.0f;
	}

	void WorldCamera::moveUpInLocal(int height /* = 10 */)
	{
		if(csys_ == CoordinateSystem::LOCAL)
		{
			target_dst_.z += height;
		}		
	}
	
	double WorldCamera::target_tilt()
	{
		return target_tilt_;
	}
	
	// 向目标位置逼近，用于每一帧更新视点位置信息
	void WorldCamera::slerpToTargetOrientation(double percent)
	{		
		if( csys_ == CoordinateSystem::GLOBAL )
		{
			if(!b_trackball_)
				return;
			double c = m_orientation_.dot(target_orientation_);
			if (c > 1.0)
				c = 1.0;
			else if (c < -1.0)
				c = -1.0;

			angle_ = acos(c);

			m_orientation_ = m_orientation_.Slerp(target_orientation_, percent);

			tilt_ += (target_tilt_ - tilt_)*percent;
			bank_ += (target_bank_ - bank_)*percent;
			distance_ += (target_distance_ - distance_)*percent;
			if( !underground_ )
			{
				if( distance_ < 3 )
					distance_ = 3;
			}
			computeAltitude();
			//fovx_ += (target_fov_ - fovx_)*percent;
		}
		else
		{
			//static int pointGoto_times = 0;
			static bool needreforce = false;
			vec3d pos;
			pos = target_dst_-local_eye_;
			float poslength = pos.length();
			if (poslength > 500)
			{
				needreforce = true;
			}
			if (needreforce)
			{
				if (poslength < 50)
				{
					forceReloadLocal();
					vec3d targetpos = earth_->ToVector3D(Geodetic3D(target_longitude_, target_latitude_, target_altitude_));
					target_dst_ = local_world2local_ * targetpos;
					pos = target_dst_-local_eye_;
					poslength = pos.length();
					needreforce = false;
				}
			}
			if(poslength < 0.01)
			{
				//coordinateSysExchange();
				/*forceReloadLocal();
				vec3d targetpos = earth_->ToVector3D(Geodetic3D(target_longitude_, target_latitude_, target_altitude_));
				vec3d target_dst_temp = local_world2local_ * targetpos;
				float newlength = (target_dst_temp - local_eye_).length();
				if (newlength > 0.01)
				{
					target_dst_ = target_dst_temp;
					pos = local_eye_ + (target_dst_ - local_eye_)*percent;
				}
				else{*/
					target_dst_ = local_eye_;
					pos = local_eye_;
				//}
				
			} else {
				pos = local_eye_ + (target_dst_ - local_eye_)*percent;
			}
			
			if(underground_)	// 判断是否允许进入地下
			{
				local_eye_ = pos;
			}
			else
			{
				if( local_eye_.z<10000)
				{
					// 判断是够与地面相交，若相交则拔高视点
					vec3d positon = local_local2world_*pos;
					Geodetic3D viewPos = earth_->ToGeodetic3D(positon);
					cur_terrain_elevation_ = 0;//wwt->GetHeight(degrees(viewPos._longitude), degrees(viewPos._latitude));
					float height = viewPos.getHeight();
					if(height > cur_terrain_elevation_ + 3.0)
					{
						local_eye_ = pos;
					}
					else
					{
						// 随地形起伏
						Geodetic3D tmpGeo(viewPos.getLongitude(),viewPos.getLatitude(),
							cur_terrain_elevation_+3.0);
						positon = earth_->ToVector3D(tmpGeo);
						local_eye_ = local_world2local_*positon;					
					}
				}
				else
					local_eye_ = pos;
			}

			local_dir_ += (target_dir_ - local_dir_)*percent;
			local_dir_ = local_dir_.normalize();
			if( fabs(local_dir_.z + 1) < 1e-6 )
			{
				vec3d tmp(sin(heading_), cos(heading_), 0);
				local_dir_ += tmp*0.02;
				local_dir_ = local_dir_.normalize();
			}

			local_target_ = local_eye_ + local_dir_;
			computeTiltHeading();
		}		
	}

	// target_altitude_赋值，须重新计算target_distance_
	void WorldCamera::set_target_altitude(double val)
	{
		if(val < terrain_elevation_)
			val= terrain_elevation_;
		if(csys_ == CoordinateSystem::GLOBAL && b_trackball_)
		{			
			target_altitude_ = val;
			computeTargetDistance();
		}
		else if(csys_ == CoordinateSystem::LOCAL)
		{
			target_dst_.z = val;
		}
		else
			return;		
	}

	void WorldCamera::setAutoMoveAndDisableSlerp(vec3d& llh)
	{
		autochange_ = llh;
		b_disableslerp = true;
	}

	// 每一帧更新相关参数信息
	void WorldCamera::update()
	{
		if( csys_ == CoordinateSystem::GLOBAL)
		{
			if(b_trackball_)
			{
				if( altitude_ < csys_change_height_ )
				{
					coordinateSysExchange();
					target_dst_ = local_eye_ + local_dir_.normalize()*
						(distance_ - target_distance_);
				}
				else
				{
					flyTo();
					if(!b_disableslerp)
					{
						if( !camera_has_momentum_ )
							slerpToTargetOrientation(0.05);
						else
							slerpToTargetOrientation( 0.2 );
					}		
					else
					{
						longitude_ += autochange_.x;
						latitude_ += autochange_.y;
						altitude_ += autochange_.z;
						computeDistance();
					}
				}
			}
			else{
				// 更新longitude_和latitude_值，此时表视点的经纬度位置
				Geodetic3D tmpGeo = earth_->ToGeodetic3D(position_);
				longitude_ = tmpGeo._longitude;
				latitude_ = tmpGeo._latitude;
				if( altitude_ < csys_change_height_ )
				{
					coordinateSysExchange();
					target_dst_ = local_eye_ + local_dir_.normalize();
				}				
			}
		}
		else if( csys_ == CoordinateSystem::LOCAL )
		{
			flyTo();
			slerpToTargetOrientation(0.05);
			Geodetic3D tmpGeo = earth_->ToGeodetic3D(position_);
			longitude_ = tmpGeo._longitude;
			latitude_ = tmpGeo._latitude;

			if( local_eye_.z > csys_change_height_+2000.0 )
				coordinateSysExchange();
			else
			{
				// 偏离坐标原点一定距离之后重新调整局部坐标系
				float dis = sqrt((local_eye_.x*local_eye_.x) + (local_eye_.y*local_eye_.y));
				if( dis > 3000.f )
				{
					forceReloadLocal();				
				}
			}
		}		
		CameraBase::update();
	}

	// 摄像机相关交互
	void WorldCamera::rotationYawPitchRoll(double yaw, double pitch, double roll)
	{
		if(csys_ == CoordinateSystem::LOCAL || !b_trackball_)
			return;
		quatd q;
		q = q.EulerToQuaternion(yaw,pitch, roll);
		target_orientation_ = q * target_orientation_;

		vec3d v = target_orientation_.QuaternionToEuler();
		target_latitude_ = v.y;
		target_longitude_ = v.x;
		if(abs(roll) > 4.94066e-324)
			target_heading_ = v.z;
	}

	// _targetDistance赋值
	void WorldCamera::set_target_distance(double val)
	{		
		if( csys_ == CoordinateSystem::GLOBAL )
		{
			if(!b_trackball_)
				return;
			if( !underground_ )
			{
				if(val < 0.01)
					val = 0.05;
			}
			else
			{
				if( val < -1000.0 )
					val = -1000.0;
			}
			target_distance_ = val;
			computeTargetAltitude();
		}
		else
		{
			target_dst_ = local_eye_ + local_dir_.normalize()*val;
			vec3d target_world = local_local2world_ * target_dst_;
			Geodetic3D target_geo = earth_->ToGeodetic3D(target_world);
			target_longitude_ = target_geo._longitude; target_latitude_ = target_geo._latitude; target_altitude_ = target_geo._height;
		}		
	}

	// 获取target_distance_
	double WorldCamera::target_distance()
	{
		return target_distance_;
	}

	// 设置目的视点坐标
	void WorldCamera::setPosition(double lat, double lon, double altitude,double tilt)
	{
		if(csys_ == CoordinateSystem::LOCAL || !b_trackball_)
			return;

		quatd q;
		target_orientation_ = q.EulerToQuaternion(lon, lat, heading_);
		vec3d v = target_orientation_.QuaternionToEuler();
		target_latitude_ = v.y;
		target_longitude_ = v.x;
		target_heading_ = v.z;
		if ( status_ == 1 )
		{
			mid_lat_ = target_latitude_;
			mid_lon_ = target_longitude_;
		}
		else if( status_ == 3 )
		{
			dst_lat_ = target_latitude_;
			dst_lon_ = target_longitude_;
		}
		target_bank_ = 0;
		if(tilt != 0)
		{
			tilt_ = tilt;
			target_tilt_ = tilt;
		}
		if(altitude != 0)
		{
			altitude_ = altitude;
			computeDistance();
			m_orientation_ = target_orientation_;
			target_distance_ = distance_;
			target_altitude_ = altitude_;
		}		
	}

	void WorldCamera::moveDirectTo(double lat, double lon, double altitude, double tilt)
	{
		if(csys_ == CoordinateSystem::LOCAL || !b_trackball_)
			return;
		quatd q;
		target_orientation_ = q.EulerToQuaternion(lon, lat, 0);
		m_orientation_ = target_orientation_;
		vec3d v = target_orientation_.QuaternionToEuler();
		target_latitude_ = v.y;
		target_longitude_ = v.x;
		target_heading_ = v.z;

		target_bank_ = 0;
		target_tilt_ = tilt;
		/*if(tilt != 0)
		{
			tilt_ = tilt;
			target_tilt_ = tilt;
		}*/
		if(altitude != 0)
		{
			altitude_ = altitude;
			computeDistance();
			m_orientation_ = target_orientation_;
			target_distance_ = distance_;
			target_altitude_ = altitude_;
		}
	}

	// 转换到非轨迹球模式时的初始化操作
	// 切换时根据当前的摄像机状态初始化各参数信息
	void WorldCamera::initFromTrackball(CameraInfo &camInfo)
	{
		if(csys_ == GLOBAL && b_trackball_)
		{
			disableTrackBall();
			/*Geodetic3D ll( longitude_, latitude_, 0 );
			vec3d startPos = this->earth_->ToVector3D( ll );		*/
			vec3d dir = getDir();
			vec3d startPos = position_ + dir;
			setTarget(startPos);
			vec3d up;
			if( abs(tilt_) > 1e-5 )
				up = this->earth_->GeodeticSurfaceNormal(position_);
			else
				up = vec3d( 0.0, 0.0, 1.0);
			setUp( up );

			camInfo.firstview_offset = vec3d(0.0, 0.0, 0.0);
			camInfo.target_pos = startPos;
			camInfo.oriview_point = position_;
			camInfo.eye_to_target = startPos - position_;
			camInfo.eye_to_target = camInfo.eye_to_target.normalize();
		}
		else if(csys_ == CoordinateSystem::LOCAL)
		{
			disableTrackBall();
			vec3d startPos = position_ + local_dir_;
			startPos = local_local2world_ * startPos;
			setTarget(startPos);
			vec3d up;
			if( abs(tilt_) > 1e-5 )
				up = this->earth_->GeodeticSurfaceNormal(position_);
			else
				up = vec3d( 0.0, 0.0, 1.0);
			setUp( up );

			camInfo.firstview_offset = vec3d(0.0, 0.0, 0.0);
			camInfo.target_pos = startPos;
			camInfo.oriview_point = position_;
			camInfo.eye_to_target = startPos - position_;
			camInfo.eye_to_target = camInfo.eye_to_target.normalize();
		}
	}

	void WorldCamera::clearALL()
	{
		enableTrackBall();
		if(csys_ == LOCAL && b_trackball_)	return;		
		Geodetic3D tmpGeo = earth_->ToGeodetic3D(position_);
		double latitude = tmpGeo._latitude;
		double longitude = tmpGeo._longitude;
		double height = tmpGeo._height;
		// 退出trackball模式时，重置摄像机参数		
		if(height < csys_change_height_)	// 进入LOCAL模式
		{
			//csys_ = CoordinateSystem::LOCAL;
			//computeLocalWorldTrans();
			//local_cenlon_ = longitude_;
			//local_cenlat_ = latitude_;
			//// 计算局部坐标系的原点
			//local_origin_ = earth_->ToVector3D(
			//	Geodetic3D(local_cenlon_, local_cenlat_, 0));
			//reference_center_ = local_origin_;
			//// 视线与地球表面交点坐标
			//local_target_ = local_world2local_ * target_;

			//local_up_ = vec3d(0, 0, 1);
			//local_eye_ = local_world2local_ * position_;
			//local_dir_ = local_target_ - local_eye_;		
			//local_dir_ = local_dir_.normalize();			
			//target_dst_ = local_eye_;
			//target_dir_ = local_dir_;

			//if( fabs(local_dir_.z + 1) < 1e-6 )
			//{
			//	vec3d tmp(sin(heading_), cos(heading_), 0);
			//	local_dir_ += tmp*0.02;
			//	local_dir_ = local_dir_.normalize();

			//	target_dir_ = local_dir_ + tmp*0.1;
			//	target_dir_ = target_dir_.normalize();
			//}
			//else
			//{
			//	target_dir_ = local_dir_;
			//}
			//local_target_ = local_eye_ + local_dir_;			
		}
		else
		{
			csys_ = GLOBAL;
			quatd q;
			target_orientation_ = q.EulerToQuaternion(longitude, latitude, heading_);
			m_orientation_ = target_orientation_;
			vec3d v = target_orientation_.QuaternionToEuler();
			target_latitude_ = v.y;
			target_longitude_ = v.x;
			target_heading_ = v.z;

			target_bank_ = 0;		
			if(tilt_ > max_tilt_)
				tilt_ = max_tilt_;

			target_tilt_ = tilt_;		
			altitude_ = height;
			computeDistance();
			target_distance_ = distance_;
			target_altitude_ = altitude_;
		}		
	}

	void WorldCamera::zoomStepped(float ticks)
	{
		double factor = 0.015;
		int currentTickCount = 100;//SDL_GetTicks();
		double minTime = 50;  // <= 50ms: fastest
		double maxTime = 250; // >=250ms: slowest
		double time = currentTickCount - last_step_zoom_tick_count;
		if (time<minTime)
			time = minTime;
		double multiplier = 1-abs( (time-minTime)/maxTime ); // Range: 1 .. 2
		if(multiplier<0)
			multiplier=0;

		multiplier= multiplier * 10.0;
		double mulfac = pow(1 - factor, multiplier+1 );
		mulfac = powf(mulfac, abs(ticks));
		if(csys_ == CoordinateSystem::GLOBAL)
		{			
			if(distance_ <0.01 && distance_>0)
				set_target_distance(0.05 );
			else 
			{
				if (ticks > 0  )
					set_target_distance(distance_ * mulfac);
				else
					set_target_distance(distance_ / mulfac);				
			}
		}
		else
		{
			// 依据地面高度计算距离
			if( ticks > 0 )
			{
				double tmp = 1.0f - mulfac;
				tmp *= fabs(local_eye_.z);
				if( tmp > 0 && tmp < 1.0 )
					tmp = 1.0;
				set_target_distance(tmp);
			}
			else
			{	
				double tmp = mulfac - 1.0f;
				tmp *= fabs(local_eye_.z);
				if( tmp < 0 && tmp > -1.0 )
					tmp = -1.0;
				set_target_distance(tmp);
			}
		}

		last_step_zoom_tick_count = currentTickCount;
	}

	/*if( ticks > 0 )
	{
		double tmp = 1.0f - mulfac;
		tmp *= fabs(local_eye_.z);
		if( tmp > 0 && tmp < 1.0 )
			tmp = 1.0;
		set_target_distance(tmp);
	}
	else
	{	
		double tmp = mulfac - 1.0f;
		tmp *= fabs(local_eye_.z);
		if( tmp < 0 && tmp > -1.0 )
			tmp = -1.0;
		set_target_distance(tmp);
	}*/

	// 通过设置target_distance_实现视点拉近或推远
	void WorldCamera::zoom(float percent)
	{
		if(percent>0)
		{
			// In
			double factor = 1.0f + percent;
			if(csys_ == GLOBAL)
				set_target_distance(target_distance_/factor);
			else
			{
				double tmp = fabs(local_eye_.z)/factor;
				if( tmp > 0 && tmp < 1.0 )
					tmp = 1.0;
				set_target_distance(tmp);
			}
		}
		else
		{
			// Out
			double factor = 1.0f - percent;
			if(csys_ == GLOBAL)
				set_target_distance(target_distance_*factor);
			else
			{
				double tmp = fabs(local_eye_.z)/factor;
				if( tmp < 0 && tmp > -1.0 )
					tmp = -1.0;				
				set_target_distance(tmp);
			}
		}
	}
	
	// 计算target_distance_--地球半径取实际半径(地球模型半径加地形高度)
	void WorldCamera::computeTargetDistance()
	{
		if(csys_ == CoordinateSystem::GLOBAL && b_trackball_)
		{
			double ang = M_PI-target_tilt_;
			double cos = std::cos(ang);
			double actualRaduis = world_radius_ + terrain_elevation_;
			double x = actualRaduis*cos;
			double hyp = world_radius_+target_altitude_;
			double y = sqrt(actualRaduis*actualRaduis*cos*cos+hyp*hyp-actualRaduis*actualRaduis);
			double res = x-y;
			if(res<0)
				res = x+y;
			target_distance_ = res;
			if(target_distance_ < 0.01)
				target_distance_ = 0.05;
		}		
	}

	// 计算target_altitude_
	void WorldCamera::computeTargetAltitude()
	{
		if(csys_ == CoordinateSystem::GLOBAL && b_trackball_)
		{
			double actualRaduis = world_radius_ + terrain_elevation_;
			double dfromeq = sqrt(actualRaduis*actualRaduis 
				+ target_distance_*target_distance_ 
				- 2 * actualRaduis*target_distance_*cos(M_PI-target_tilt_));
			double alt = dfromeq - world_radius_;

			if(alt > maximum_altitude_)
				alt = maximum_altitude_;
			target_altitude_ = alt;
		}		
	}

	void WorldCamera::flyTarget(double lat, double lon, double height, double tilt )
	{
		if(csys_ == GLOBAL && !b_trackball_)
			return;
		status_ = 1;
		dst_lat_ = lat;
		dst_lon_ = lon;
		mid_lat_ = (lat + latitude_) / 2;
		mid_lon_ = (lon + longitude_) / 2;
		dst_height_ = height;
		target_tilt_ = tilt;
		if (csys_ == LOCAL)
		{
			double newtilt = tilt * 0.5;
			target_dir_ = vec3d(0, sin(tilt), cos(-tilt));

		}
	}

	void WorldCamera::flyTo()
	{
		if( csys_ == LOCAL && status_ == 1 )
		{
			target_dst_ = local_eye_ + vec3d(0,0,5000);	
			if( target_dir_.z >= -0.2 )
			{
				target_dir_.z = -0.5;
				target_dir_ = target_dir_.normalize();
			}
		}
		else if(status_)
		{
			if (status_ == 1)
			{
				heading_ = target_heading_ = heading_ * 0.5;
				setPosition(mid_lat_, mid_lon_, 0, 0 );
				double tmp_dis = ( fabs(latitude_ - dst_lat_) * 180.0f / M_PI + 
					fabs(longitude_ - dst_lon_) * 180.0f / M_PI ) * 210000;
				if(tmp_dis > world_radius_) tmp_dis = world_radius_;
				//tmp_dis /= 2;
				double zoom;
				if( altitude_ < tmp_dis )
					set_target_altitude( tmp_dis );
				double mid_tilt = (target_tilt_ - tilt_) * 0.5 + tilt_;
				set_target_tilt( mid_tilt );
				
				status_ = 2;
			}
			else if(status_ == 2)
			{
				if (abs(latitude_ - mid_lat_) < 0.0002 && abs(longitude_ - mid_lon_) < 0.0002)
				{
					status_ = 3;
				}
			}
			else if(status_ == 3)
			{
				heading_ = target_heading_ = 0;
				setPosition(dst_lat_, dst_lon_, 0.0, 0.0);
				double zoom;
				set_target_altitude(dst_height_);
				set_target_tilt(target_tilt_);
				
				status_ = 0;
			}
		}
	}

	double WorldCamera::target_fov()
	{
		return target_fov_;
	}

	vec3d WorldCamera::target_dir()
	{
		return target_dir_;
	}

	vec3d WorldCamera::target_dst()
	{
		return target_dst_;
	}

	void WorldCamera::set_target_dir(const vec3d &target_dir)
	{
		if(csys_ != CoordinateSystem::LOCAL)	return;
		target_dir_ = target_dir;
		target_dir_ = target_dir_.normalize();
	}

	void WorldCamera::set_target_dst(const vec3d &target_dst)
	{
		if(csys_ != CoordinateSystem::LOCAL)
			return;
		target_dst_ = target_dst;
		vec3d target_world = local_local2world_ * target_dst_;
		Geodetic3D target_geo = earth_->ToGeodetic3D(target_world);
		target_longitude_ = target_geo._longitude; target_latitude_ = target_geo._latitude; target_altitude_ = target_geo._height;
	}

	bool WorldCamera::camera_has_momentum()
	{
		return camera_has_momentum_;
	}

	void WorldCamera::set_camera_has_momentum(bool camera_has_momentum)
	{
		camera_has_momentum_ = camera_has_momentum;
	}

	double WorldCamera::target_latitude()
	{
		return target_latitude_;
	}

	double WorldCamera::target_longitude()
	{
		return target_longitude_;
	}

	double WorldCamera::target_altitude()
	{
		return target_altitude_;
	}

	void WorldCamera::setEye(const vec3d &eye)
	{
		if(csys_ == GLOBAL){
			if(!b_trackball_)
			{
				if(!underground_)	// 不允许进入地下
				{
					Geodetic3D tmpGeo = earth_->ToGeodetic3D(eye);
					double height = 0;//wwt->GetHeight(degrees(tmpGeo._longitude), degrees(tmpGeo._latitude));
					if(tmpGeo._height < height + 3)
					{
						position_ = earth_->ToVector3D(Geodetic3D(tmpGeo._longitude, tmpGeo._latitude,
							height + 3));
					}
					else
						position_ = eye;
				}
				else
				{
					position_ = eye;
				}
			}
			else return;
		}		
		else{
			local_eye_ = local_world2local_*eye;
			target_dst_ = local_eye_;
		}
	}

	void WorldCamera::setTarget(const vec3d &target)
	{
		if(csys_ == GLOBAL){
			if(b_trackball_)
				return;
			else
			{				
				target_ = target;
			}
		}
		else
		{
			// should setEye first
			local_target_ = local_world2local_ * target;
			local_dir_ = local_target_ - local_eye_;
			local_dir_ = local_dir_.normalize();
			target_dir_ = local_dir_;
		}
	}

	void WorldCamera::pointGoto(double lat, double lon, double height /* = -100000.0 */)
	{
		if(csys_ == CoordinateSystem::GLOBAL && b_trackball_)
		{
			if (height < -99999.0)
			{
				setPosition(lat, lon, 0, 0);
			}else{
				setPosition(lat, lon, height, 0);
			}
			
		}
		else if(csys_ == CoordinateSystem::LOCAL)
		{
			target_longitude_ = lon; target_latitude_ = lat; 
			if (height < -99999.0)
			{
				target_altitude_ = altitude();
			}else{
				target_altitude_ = height;
			}
			Geodetic3D target_pos(target_longitude_, target_latitude_, target_altitude_);
			vec3d pos = earth_->ToVector3D(target_pos);
			vec3d tmp = local_world2local_ * pos;	
			//in LOCAL mode, only chage the xy which stand for longitude and latitude
			target_dst_.x = tmp.x;
			target_dst_.y = tmp.y;
		}
		else return;
	}

	MomentumCamera::MomentumCamera(Ellipsoid* ellipsoid) : 
			WorldCamera(ellipsoid)
	{
		target_orientation_ = m_orientation_;
		target_distance_ = distance_;
		target_altitude_ = altitude_;
		target_tilt_ = tilt_;
		//cameraHasMomentum = false;
		heading_momentum_ = latitude_momentum_ = longitude_momentum_ = 0.0f;
	}
	MomentumCamera::~MomentumCamera()
	{

	}
	void MomentumCamera::rotationYawPitchRoll(double yaw, double pitch, double roll)
	{
		if(csys_ == CoordinateSystem::LOCAL || !b_trackball_)
			return;
		if(camera_has_momentum_)
		{
			latitude_momentum_ += pitch/100;
			longitude_momentum_ += yaw/100;
			heading_momentum_ += roll/100;
		}
		quatd q;
		/*_targetOrientation = quatd(-0.236204166, 0.13490817, 0.66988, 0.69083);
		roll = 0.05045;*/
		target_orientation_ = q.EulerToQuaternion( yaw, pitch, roll) * target_orientation_;
		vec3d v = target_orientation_.QuaternionToEuler();
		target_latitude_ = v.y;
		target_longitude_ = v.x;
		//_targetHeading = v.z;
		WorldCamera::rotationYawPitchRoll(yaw,pitch,roll);
		vec3d p = target_orientation_.QuaternionToEuler();
		double t0 = latitude_ - p.y;
		double t1 = longitude_ - p.x;
	}

	void MomentumCamera::setPosition(double lat, double lon, double alt, double tilt)
	{
		if(csys_ == CoordinateSystem::LOCAL || !b_trackball_)
			return;
		latitude_momentum_ = 0;
		longitude_momentum_ = 0;
		heading_momentum_ = 0;
		WorldCamera::setPosition(lat,lon,alt,tilt);
	}
	void MomentumCamera::setPosition(double lat, double lon)
	{
		if(csys_ == CoordinateSystem::LOCAL || !b_trackball_)
			return;
		latitude_momentum_ = 0;
		longitude_momentum_ = 0;
		heading_momentum_ = 0;
		WorldCamera::setPosition(lat,lon,0,0);
	}
	void MomentumCamera::update()
	{
		static int c = 0;
		if(camera_has_momentum_)
		{
			if( c++ < 20 )
			{
				WorldCamera::rotationYawPitchRoll(longitude_momentum_,
					latitude_momentum_,	heading_momentum_);
			}
			else 
			{
				c = 0;
				camera_has_momentum_ = false;
			}
		}
		WorldCamera::update();
	}
	void MomentumCamera::pan(double lat, double lon)
	{
		if(csys_ == CoordinateSystem::LOCAL || !b_trackball_)
			return;
		if(camera_has_momentum_)
		{			
			const float _lat = lat / (fabs( lat ) + fabs( lon ));
			const float _lon = lon / (fabs( lat ) + fabs( lon ));
			latitude_momentum_ = speed_ * _lat / 100000;
			longitude_momentum_ = speed_ * _lon / 100000;
		}
		else
		{
			lat += target_latitude_;
			lon += target_longitude_;

			if(abs(lat)>M_PI/2-1e-3)
			{
				if(lat >0)
					lat = M_PI/2 - 1e-3;
				else
					lat = -(M_PI/2 - 1e-3);
			}
			quatd q;
			target_orientation_ = q.EulerToQuaternion(
				lon,
				lat,
				target_heading_);

			vec3d v = target_orientation_.QuaternionToEuler();
			target_latitude_ = v.y;
			target_longitude_ = v.x;
			target_heading_ = v.z;
		}			
	}
	
	// LOCAL模式下计算_tilt和_heading
	void WorldCamera::computeTiltHeading()
	{
		if(csys_ != CoordinateSystem::LOCAL)
			return;
		// _heading
		double x,y;
		x = local_dir_.x;
		y = local_dir_.y;
		double z;
		z = y/sqrt(x*x+y*y);
		if( x >= 0 )
			heading_ = acos(z);
		else
			heading_ = -acos(z);

		// _tilt
		z = -local_dir_.z;
		tilt_ = acos(z);
	}

	void WorldCamera::coordinateSysExchange()
	{
		if(csys_ == CoordinateSystem::GLOBAL)
		{
			csys_ = CoordinateSystem::LOCAL;

			computeLocalWorldTrans();
			local_cenlon_ = longitude_;
			local_cenlat_ = latitude_;

			// 计算局部坐标系的原点
			local_origin_ = earth_->ToVector3D(
				Geodetic3D(local_cenlon_, local_cenlat_, 0));
			reference_center_ = local_origin_;
			// 视线与地球表面交点坐标
			if(b_trackball_){
				local_target_ = vec3d(0.0f, 0.0f, 0.0f);
			}
			else {
				local_target_ = local_world2local_*target_;
			}
			
			local_up_ = vec3d(0, 0, 1);
			local_eye_ = local_world2local_ * position_;
			local_dir_ = local_target_ - local_eye_;		
			local_dir_ = local_dir_.normalize();

			if( fabs(local_dir_.z + 1) < 1e-6 )
			{
				vec3d tmp(sin(heading_), cos(heading_), 0);
				local_dir_ += tmp*0.02;
				local_dir_ = local_dir_.normalize();

				target_dir_ = local_dir_ + tmp*0.1;
				target_dir_ = target_dir_.normalize();
			}
			else
			{
				target_dir_ = local_dir_;
			}
			local_target_ = local_eye_ + local_dir_;			
		}
		else if(csys_ == LOCAL)
		{	
			if(b_trackball_){
				vec3d vec(0, 0, -1);
				double tmp = vec.dotproduct(local_dir_);
				if( tmp <= 0 )	// 视线与地面有交点
				{
					local_dir_.z = -0.7;				
				}
				position_ = local_local2world_*local_eye_;
				vec3d dir = local_dir_.normalize();
				double k = -local_eye_.z/dir.z;
				vec = local_eye_ + dir*k;
				vec = local_local2world_*vec;
				target_ = vec;
				Geodetic3D geo = earth_->ToGeodetic3D(vec);
				latitude_ = geo.getLatitude();
				longitude_ = geo.getLongitude();
				distance_ = fabs(k);
				target_distance_ = fabs(target_dst_.z/dir.z);

				tmp = 0.0;
				/*if(wwt)
				{
					tmp = wwt->GetHeight(degrees(longitude_), degrees(latitude_));
				}*/

				distance_ -= tmp;
				target_distance_ -= tmp;
				computeTiltHeading();
				target_heading_ = heading_;
				target_tilt_ = tilt_;

				quatd q;
				target_orientation_ = 
					q.EulerToQuaternion(longitude_, latitude_, heading_);
				m_orientation_ = target_orientation_;
			}
			else{
				target_ = local_world2local_ * local_target_;
				vec3d up;
				if( abs(tilt_) > 1e-5 )
					up = this->earth_->GeodeticSurfaceNormal(position_);
				else
					up = vec3d( 0.0, 0.0, 1.0);
				setUp( up );				
			}
					
			csys_ = CoordinateSystem::GLOBAL;
		}
	}

	void WorldCamera::forceReloadLocal()
	{
		// 以相机位置处为原点
		position_ = local_local2world_*local_eye_;
		vec3d target = local_eye_ + local_dir_*100;
		target = local_local2world_*target;
		vec3d x = local_local2world_*target_dst_;

		Geodetic3D geo = earth_->ToGeodetic3D(position_);
		longitude_ = geo.getLongitude();
		latitude_ = geo.getLatitude();
		computeLocalWorldTrans();
		local_cenlon_ = longitude_;
		local_cenlat_ = latitude_;
		local_origin_ = earth_->ToVector3D(
			Geodetic3D(local_cenlon_, local_cenlat_, 0));
		reference_center_ = local_origin_;

		local_target_ = local_world2local_*target;
		target_dst_ = local_world2local_*x;

		local_eye_ = local_world2local_*position_;
		local_dir_ = local_target_ - local_eye_;
		local_dir_ = local_dir_.normalize();
		if( fabs(local_dir_.z + 1) < 1e-6 )
		{
			vec3d tmp(sin(heading_)*0.01, cos(heading_)*0.01, 0);
			local_dir_ += tmp;
			local_dir_ = local_dir_.normalize();
		}
		local_target_ = local_eye_ + local_dir_;
	}

	void WorldCamera::cameraMoveUp(float dist)
	{
		if(csys_ != CoordinateSystem::LOCAL)	return;			
		if(dist <= 0)	return;
		local_eye_.z += dist;
		target_dst_ = local_eye_;
	}

	void WorldCamera::cameraMoveDown(float dist)
	{
		if(csys_ != CoordinateSystem::LOCAL)	return;			
		if(dist <= 0)	return;
		local_eye_.z -= dist;
		target_dst_ = local_eye_;
	}

	void WorldCamera::cameraMoveForward(float dist)
	{
		if(csys_ != CoordinateSystem::LOCAL)	return;			
		if(dist <= 0)	return;
		vec3d dir = local_dir_;
		dir.z = 0;
		dir = dir.normalize();
		local_eye_ += dir * dist;
		target_dst_ = local_eye_;
	}

	void WorldCamera::cameraMoveBackward(float dist)
	{
		if(csys_ != CoordinateSystem::LOCAL)	return;			
		if(dist <= 0)	return;
		vec3d dir = local_dir_;
		dir.z = 0;
		dir = dir.normalize();
		local_eye_ += dir * dist;
		target_dst_ = local_eye_;
	}

}
