/**
* @file worldCamera.h 
* @brief 本文件包含两个摄像机的继承类,WorldCamera和MomentumCamera.
*
* @details WorldCamera类继承自CameraBase，MomentumCamera类继承自WorldCamera, \n
* 主要用于描述和计算摄像机的运动状态信息,用户可以通过建立这两个类的对象，来实\n
* 现对摄像机的调度.其中，MomentumCamera还可以实现保持摄像机的动量 \n
* 由于两种交互模式的存在，不同的操作可能适用于不同的模式,仅适用于某种模式的将会标出.
*/
#ifndef _VGS_WORLDCAMERA_H_
#define _VGS_WORLDCAMERA_H_

#include "CameraBase.h"

namespace VirtualGlobeScene
{
	/**
	* @class WorldCamera
	* @brief 继承自CameraBase，实现摄像机的调度.
	* @details 通过设置一些目标状态，比如target_distance_,target_altitude_,target_tilt_, \n
	* target_orientation_等用于GLOBAL模式，以及target_dst,target_dir等用于LOCAL模式，  \n
	* 通过这些参数来描述摄像机的目标状态，并且从当前的状态不停地向目标状态逼近，来实  \n
	* 现摄像机的运动，完成对摄像机的调度.
	*/
	class WorldCamera : public CameraBase
	{
	public:
		/**
		* @brief 构造函数，初始化相关信息，ellipsoid和win用来初始化基类.
		*/
		WorldCamera(Ellipsoid *ellipsoid);

		virtual ~WorldCamera();
		
		/**
		* @brief target_tilt_赋值
		*/
		void set_target_tilt(double t);
		/**
		* @brief 获取target_tilt_的值
		*/
		double target_tilt();

		/**
		* @brief 更新状态信息，向目标状态逼近
		* @param percent表由当前状态向目标状态逼近的程度，取值（0,1）
		*/
		void slerpToTargetOrientation(double percent);
		
		/**
		* @brief 用于每帧更新摄像机的状态信息.
		*/
		virtual void update();
		
		/**
		* @brief 旋转摄像机
		* 
		* @param yaw航向
		* @param pitch俯仰
		* @param roll横滚
		*/
		void rotationYawPitchRoll(double yaw, double pitch, double roll);

		/**
		* @brief 计算target_altitude_值
		* @details 根据target_distance_和target_tilt_计算target_altitude_值，\n
		* 常用于target_distance_或者target_tilt_更新后重新计算target_altitude_.
		*/
		void computeTargetAltitude();

		/**
		* @brief target_altitude赋值
		*/
		void set_target_altitude(double val);

		/**
		* @brief 设置自动移动的尺度并终止Slerp模式
		* @param llh传递给_atuoChange，表移动尺度
		*/
		void setAutoMoveAndDisableSlerp(vec3d &llh);

		/**
		* @brief 计算target_distance_值
		* @details 根据target_altitude_和target_tilt_计算target_distance_值，\n
		* 常用于target_altitude_或者target_tilt更新后重新计算target_distance_.
		*/
		void computeTargetDistance();

		/**
		* @brief 沿视线方向推进或者拉远
		* @param percent表推进或者拉远的比例，正值表推进（目标视点高度变为 \n
		* 原来的1.0/(1+percent)）,负值表拉远(目标视点高度变为原来的1.0*(1-percent)倍).
		*/ 
		void zoom(float percent);

		/**
		* @brief 沿视线方向拉近或者推远摄像机,可用于滚轮交互
		* @param ticks表示移动幅度，正值拉近，负值推远.
		*/
		void zoomStepped(float ticks);

		/**
		* @brief target_distance_赋值
		*/
		void set_target_distance(double val);
		/**
		* @brief 获取_targetDistance值
		*/
		double target_distance();

		/**
		* @brief 设置目标视点位置
		* @param lat目标纬度
		* @param lon目标经度
		* @param altitude目标视点高度
		* @param tilt目标摄像机视线反向与视线和地表交点的夹角
		*/
		virtual void setPosition(double lat, double lon, double altitude, double tilt);

		/**
		* @brief 将视点移动到目标位置
		* @param lat 目标位置纬度
		* @param lon 目标位置经度
		* @param height target height(default is -100000, if height is -100000,  target height would be the current camera altitude)
		*/
		virtual void pointGoto(double lat, double lon, double height = -100000.0);

		/**
		* @brief 垂直抬高视点的位置
		* @param height 视点抬高的距离
		*/
		void moveUpInLocal(int height = 100);
		
		/**
		* @brief 矫正视点位置，在当前经纬度下拉高视点并朝北正视地面
		*/
		void reLocate();

		/**
		* @brief 矫正视点位置，使顶切面朝北
		*/
		void resetHead();

		/**
		* @brief 完成指定飞行任务，在update中每帧监控并更新飞行状态
		*/ 
		void flyTo();

		/**
		* @brief 设置摄像机飞行目标位置
		* @param lat飞行目标纬度（弧度值）
		* @param lon飞行目标经度（弧度值）
		* @param height飞行目标高度，默认值6000，表海平面之上的高度
		* @param tilt 视点水平角度
		*/
		void flyTarget(double lat, double lon, double height = 6000.0f, double tilt = 0.0f );

		/**
		* @brief 摄像机直接切换到目标位置，无飞行过程
		* @param lat目标位置纬度
		* @param lon目标位置经度
		* @param altitude目标位置高度
		* @param tilt目标tilt值
		*/
		void moveDirectTo(double lat, double lon, double altitude, double tilt = 0.0f);

		/**
		* @brief 切换摄像机的模式，LCOAL转换成GLOBAL，或者反之
		* @details 依据摄像机的高度进行切换，高于某一值时为GLOBAL模式，否则切换到LOCAL模式，
		* 切换过程中进行相关参数的初始化
		*/
		void coordinateSysExchange();

		/************************************************************************/
		/* Local模式下强制 重新设置新的Local坐标系 包括距离移动过远、自动飞行漫游期间                                                                     */
		/************************************************************************/
		void forceReloadLocal();
		/**
		* @brief _position赋值
		*/
		void setEye(const vec3d &eye);

		/**
		* @brief _target赋值
		*/
		void setTarget(const vec3d &target);

		/**
		* @brief LOCALM模式下计算_tilt和_heading值
		*/
		void computeTiltHeading();

		/**
		* @brief 获取target_fov_值
		*/
		double target_fov();

		/**
		* @brief 获取target_dir_值
		*/
		vec3d target_dir();

		/**
		* @brief 获取target_dst_值
		*/
		vec3d target_dst();

		/**
		* @brief target_dst_赋值
		*/
		void set_target_dst(const vec3d &target_dst);

		/**
		* @brief target_dir_赋值
		*/
		void set_target_dir(const vec3d &target_dir);

		/**
		* @brief camera_has_momentum赋值
		*/
		void set_camera_has_momentum(bool camera_has_momentum);

		/**
		* @brief 获取camera_has_momentum_
		*/
		bool camera_has_momentum();

		/**
		* @brief 获取target_longitude_
		*/
		double target_longitude();

		/**
		* @brief 获取target_latitude_
		*/
		double target_latitude();

		/**
		* @brief 获取target_altitude_
		*/
		double target_altitude();

		/**
		* @brief 轨迹球模式转换到非轨迹球模式时进行初始化操作
		* @param camInfo引用类型，用来记录摄像机交互相关信息
		*/
		void initFromTrackball(CameraInfo &camInfo);

		/**
		* @brief 退出非轨迹球模式
		*/
		void clearALL();

		/**
		* @brief LOCAL模式下，摄像机垂直上移
		* @param dist 移动的距离，单位为米
		*/
		void cameraMoveUp(float dist);

		/**
		* @brief LOCAL模式下，摄像机垂直下移
		* @param dist 移动的距离，单位为米
		*/
		void cameraMoveDown(float dist);

		/**
		* @brief LOCAL模式下，摄像机沿视线方向水平向前移动
		* @param dist 移动的距离，单位为米
		*/
		void cameraMoveForward(float dist);

		/**
		* @brief LOCAL模式下，摄像机沿视线方向水平向后移动
		* @param dist 移动的距离，单位为米
		*/
		void cameraMoveBackward(float dist);


	protected:
		quatd target_orientation_;		///< 目标四元数坐标
		double target_latitude_;		///< 目标维度
		double target_longitude_;		///< 目标经度
		double target_altitude_;		///< 目标高度
		double target_distance_;		///< 目标距离
		double target_heading_;			///< 目标方位
		double target_bank_;			///<
		double target_tilt_;			///< 目标视线方向（视线方向与地球表面交点法相的夹角）
		double target_fov_;				///<
		bool camera_has_momentum_;		///< bool型变量标记是否保持动量

	private:		
		double angle_;					///<
		bool b_disableslerp;			///< 
		vec3d autochange_;				///< 		
		// 用于LCOAL 模式
		vec3d target_dst_;		///< LOCAL模式下，视点目标坐标
		vec3d target_dir_;		///< LOCAL模式下，视线目标方向
		// 用于飞行到目的地
		double dst_lat_;		///< 飞行目的地纬度
		double dst_lon_;		///< 飞行目的地经度
		double mid_lat_;		///< 中间过渡纬度
		double mid_lon_;		///< 中间过渡经度
		double dst_height_;	///< 飞行目的地的高度

		/**
		* @brief 描述飞行状态
		* @details 0--起飞 1--设置目的地为过渡地带 \n 
		* 2--飞行 3--设置目的地为终点 4--飞行.
		*/
		int status_;
	};

	/**
	* @class MomentumCamera
	* @brief 继承自WorldCamera,描述带动量的摄像机.
	*/
	class MomentumCamera : public WorldCamera
	{
	public:
		/**
		* @brief 构造函数，参数用于初始化基类
		*/
		MomentumCamera(Ellipsoid* ellipsoid);
		virtual ~MomentumCamera();
		/**
		* @brief 旋转摄像机
		* @details yaw航向，pitch俯仰，roll横滚，如果有动量的话，初始化动量值
		*/
		void rotationYawPitchRoll(double yaw, double pitch, double roll);
		/**
		* @brief 移动一定的经纬度
		* @param lat待移动的纬度
		* @param lon待移动的经度
		*/
		void pan(double lat, double lon);
		
		/**
		* @brief 每帧更新状态信息
		*/
		virtual void update();

		/**
		* @brief 设置目标视点位置
		* @details lat,lon分别是目标视点纬度和经度
		*/
		virtual void setPosition(double lat, double lon);

		/**
		* @brief 设置目标视点位置
		* @details lat,lon分别是目标视点纬度和经度，alt为高度值，tilt为视角
		*/
		virtual void setPosition(double lat, double lon, double alt =0,double tilt =0);		
	protected:
		double latitude_momentum_;	///< 纬度动量
		double longitude_momentum_;	///< 经度动量
		double heading_momentum_;	///< 朝向动量
	private:
		double speed_;	///< 用于控制移动速度
	};
}


#endif
