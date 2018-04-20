/**
* @file CameraBase.h 
* @brief 摄像机基类，描述摄像机当前状态，并计算相关矩阵.
* @details 摄像机包括两种模式GLOBAL和LCOAL，依据特定视点高度H（默认10000）切换模式  \n
* 高于H时使用GLOBAL模式，低于H则使用LOCAL模式 \n
* GLOBAL模式，坐标系以地球球心为原点，北极方向为z轴，xy平面为赤道面 \n
* GLOBAL模式摄像机状态的描述：以视线与地表交点O为基准点，通过视点到O的距离distance_,\n
* 视线反向与O点法相的夹角tilt_，摄像机的朝向heading_来描述 \n
* LOCAL模式，坐标系以局部地表某一经纬度处为原点，x轴指正东，y轴指正北,z轴为地表法相 \n
* LOCAL模式摄像机的状态的描述：通过up值local_up，视线方向local_dir，视点位置local_eye来描述.
*/
#ifndef _VGS_CAMERABASE_H_
#define _VGS_CAMERABASE_H_

#include "../math/vec3.h"
#include "../math/vec4.h"
#include "../math/quat.h"
//#include "../core/GlEnv.h"
#include "../Geometry/Ellipsoid.h"

#define dmax(a,b)    (((a) > (b)) ? (a) : (b))
#define dmin(a,b)    (((a) < (b)) ? (a) : (b))

using namespace VirtualGlobeCore;

namespace VirtualGlobeScene
{
	class WorldWindTerrainSource;
	/** 
	* @brief 用于记录局部坐标系下交互信息
	*/
	struct CameraInfo
	{
		vec3d firstview_offset;
		vec3d oriview_point;
		vec3d target_pos;
		vec3d eye_to_target;
	};

	/**
	* @brief 枚举型变量，用于表示描述当前摄像机模式
	*/
	enum CoordinateSystem
	{
		LOCAL,	///< 局部坐标系模式
		GLOBAL	///< 全局坐标系模式
	};

	enum Screen
	{
		LEFT,
		MIDDLE,
		RIGHT,
		FULL
	};
	/**
	* @class CameraBase
	* @brief 摄像机基类
	*/
	class CameraBase
	{
	public:
		/**
		* @brief 构造函数，初始化相关信息
		*
		* @param ellipsoid是Ellipsoid类指针，描述地球模型信息
		* @param win指示绘制窗口
		*/
		CameraBase(Ellipsoid* ellipsoid);
		friend class StereoImplement; ///< 友元实现立体模式
		virtual ~CameraBase();

		/**
		* @brief tilt_赋值
		* 
		* @param t为目标tilt_值
		*/
		virtual void set_tilt(double t);

		/**
		* @brief 获取tilt_值
		*/
		virtual double tilt();

		/**
		* @brief 计算altitude_值
		* @details 根据distance_和tilt_计算altitude_值，\n
		* 常用于distance_或者tilt_更新后重新计算altitude_.
		*/
		bool computeAltitude();

		/**
		* @brief 计算distance_值
		* @details 根据altitude_和tilt_计算distance_值，\n
		* 常用于altitude_或者tilt_更新后重新计算distance_.
		*/
		void computeDistance();

		/**
		* @brief 计算_tilt值
		* @details 根据alt(视点在海平面上的高度)和 \n
		* _distance（视点到视线与地球表面交点的距离）计算_tilt值，\n
		* 常用于_altitude或者_distance更新后重新计算_tilt.
		*/
		void computeTilt();

		/**
		* @brief 获取视点在地表上的高度
		*/
		double altitudeAboveTerrain();

		/**
		* @brief 为distance_赋值val
		*/
		void set_distance(double val);

		/**
		* @brief 获取distance_值
		*/
		double distance();

		/**
		* @brief 将视点移动到目标位置
		* @param lat 目标位置纬度
		* @param lon 目标位置经度
		*/
		//virtual void pointGoto(double lat, double lon);

		/**
		* @brief 设置视点位置到特定经纬度，继承类中实现
		*/
		virtual void setPosition(double lat, double lon);		
		
		/**
		* @brief 计算矩阵，并赋值给m_absolute_view_matrix_.
		*/
		virtual void computeAbsoluteMatrices();
		
		/**
		* @brief 计算ViewMatrix矩阵，并赋值给m_view_matrix_.
		*/		
		virtual void computeViewMatrix();

		/**
		* @brief 获取m_view_matrix()
		*/
		mat4f m_view_matrix();

		/**
		* @brief 获取m_world_matrix_
		*/
		mat4d m_world_matrix();

		/**
		* @brief 获取m_absolute_projection_matrix_
		*/
		mat4f m_absolute_projection_matrix();

		/**
		* @brief 获取m_absolute_view_matrix_
		*/
		mat4f m_absolute_view_matrix();

		/**
		* @brief 获取reference_center_
		*/
		vec3d reference_center();

		/**
		* @brief underground_赋值
		*/
		void set_underground(bool underground);

		/**
		* @brief 获取underground_
		*/
		bool underground();

		/**
		* @brief 获取position_
		*/
		vec3d position();

		/**
		* @brief 获取heading_
		*/
		double heading();

		/**
		* @brief 获取altitude_
		*/
		double altitude();

		/**
		* @brief 获取world_radius_
		*/
		double world_radius();

		/**
		* @brief 获取up_
		*/
		vec3d up();

		/**
		* @brief 获取target_
		*/
		vec3d target();

		/**
		* @brief 获取sundir_
		*/
		vec3d sundir();

		/**
		* @brief sundir_赋值
		*/
		void set_sundir(const vec3d& sundir);

		/**
		* @brief 获取csys_
		*/
		CoordinateSystem csys();

		/**
		* @brief 获取local_world2local_
		*/
		mat4d local_world2local();

		/**
		* @brief 获取perspective_far_plane_distance_
		*/
		float perspective_far_plane_distance();

		/**
		* @brief 获取perspective_near_plane_distance_
		*/
		float perspective_near_plane_distance();
		
		/**
		* @brief 获取longitude_
		*/
		double longitude();

		/**
		* @brief 获取latitude_
		*/
		double latitude();

		/**
		* @brief 获取fovx_
		*/
		double fovx();
		/**
		* @brief 设置横向fovx_
		*/
		void setFovx(double fov);
		/**
		* @brief 获取aspect_
		*/
		double aspect();
		
		/**
		* @brief 获取fovy_
		*/
		double fovy();

		/**
		* @brief 获取cureast_
		*/
		vec3d cureast();

		/**
		* @brief 获取cureast_
		*/
		vec3d curnorth();

		/**
		* @brief 获取cureast_
		*/
		vec3d curup();

		/**
		* @brief 获取csys_change_height_
		*/
		float csys_change_height();

		/**
		* @brief 获取terrain_elevation_
		*/
		short terrain_elevation();

		/**
		* @brief 获取height_above_terrian_
		*/
		double height_above_terrian();


		/**
		* @brief 获取local_local2world_
		*/
		mat4d local_local2world();
		
		/**
		* @brief 获取local_up_
		*/
		vec3d local_up();

		/**
		* @brief 获取local_dir_
		*/
		vec3d local_dir();

		/**
		* @brief 获取local_eye_
		*/
		vec3d local_eye();

		/**
		* @brief 获取local_origin_
		*/
		vec3d local_origin();
		/**
		* @brief 获取b_trackball
		*/
		bool b_trackball();

		/**
		* @brief 获取viewport_
		*/
		vec4i viewport();		

		/**
		* @brief viewport_赋值
		*/
		void set_viewport(const vec4i &viewport);		

		/**
		* @brief 用于每帧更新摄像机状态并计算相关参数
		*/
		virtual void update();

		/**
		* @brief 用于轨迹球模式下每帧更新摄像机状态并计算相关参数
		*/
		void updateTrackBall();

		/**
		* @brief 用于非轨迹球模式下每帧更新摄像机状态并计算相关参数
		*/
		void updateNonTrackBall();

		/**
		* @brief 用于LOCAL模式下每帧更新摄像机状态并计算相关参数
		*/
		void updateLocal();
		
		/**
		* @brief 根据屏幕坐标反求经纬度值
		* @param latitude引用类型，用于获取结果纬度值
		* @param longitude引用类型，用于获取结果经度值
		*/
		virtual bool pickingRayIntersection(int screenx, int screeny, double& latitude,
		double& longitude);
		
		/**
		* @brief 根据屏幕坐标反求世界坐标
		* @param v为屏幕坐标，z值表深度
		* @return 返回屏幕坐标点在地球球心世界坐标下的坐标值
		*/
		vec3d unproject(const vec3d& v);

		/**
		* @brief 获取全局坐标系下的视线方向
		*/
		vec3d getDir();
		
		/**
		* @brief 更新frustumVal
		*/
		void updateFrustum(float* modl,float* proj);
		
		/**
		* @brief 
		*/
		int cubeInFrustum(vec3f *cube);
		
		/**
		* @brief 
		*/
		bool sphereInFrustum(const vec3d &center, float radius);
		
		/**
		* @brief 非轨迹球模式下计算_tilt和_heading值
		*/
		void computeTiltAndHeading();		
		
		/**
		* @brief 获取方位信息 \n
		* 通过引用返回结果，up（上），north（北），east（东）.
		*/
		void getCurDir(vec3d &up, vec3d &north, vec3d &east);

		/**
		* @brief 获取视点地理位置信息，包括经纬度和高度.
		*/
		Geodetic3D getEyeGeo();

		/**
		* @brief 获取摄像机的up向量，在世界坐标系下.
		*/
		vec3d getCameraUpVector();

		/**
		* @brief 获取摄像机在局部坐标下的up向量
		*/
		vec3d getPartCsysUp();

		/**
		* @brief 获取摄像机在局部坐标下的视线方向
		*/
		vec3d getPartCsysDir();

		/**
		* @brief 获取局部坐标向世界坐标转换的矩阵
		*/
		mat4f getLocalToWorldTransform();		

		/**
		* @brief wwt赋值，形参wwt指向地形资源
		*/
		//void setWWTSource(VirtualGlobeScene::WorldWindTerrainSource* wwt);

		// 以下非轨迹球模式
		/**
		* @brief 切换到非轨迹球模式
		*/
		void disableTrackBall();

		/**
		* @brief 切换到轨迹球模式
		*/
		void enableTrackBall();
		
		/**
		* @brief _up赋值
		*/
		void setUp(const vec3d &up);		

		/**
		* @brief 计算LOCAL模式下，局部坐标和世界坐标相互转换的矩阵 \n
		* 结果存储在local2world和world2local矩阵中.
		*/
		void computeLocalWorldTrans();

		/**
		* @brief csys_赋值
		*/
		void setCsys(CoordinateSystem mode);	
		void setScr(Screen mode)
		{
			scr = mode;
		}

		long getScr()
		{
			return scr;
		}

		void setNearFar(bool is_auto, float n, float f)
		{
			b_is_auto = is_auto;
			if (!b_is_auto)
			{
				perspective_near_plane_distance_ = n;
				perspective_far_plane_distance_ = f;
			}
		}

	protected:
		double	world_radius_;				///< 地球模型半径
		quatd	m_orientation_;				///< 四元数表示当前摄像机状态
		double	heading_;					///< 指示视线方向（东南西北）
		double	latitude_;					///< 视线与地表交点的经度值
		double	longitude_;					///< 视线与地表交点的纬度值
		double	altitude_;					///< 视点高度（以海平面为0基准）
		double	distance_;					///< 视点到视线与地表交点的距离
		double	tilt_;						///< 视线和视线与地表交点法相的夹角
		double	fovx_,fovy_,aspect_;		///<
		double	bank_;						///<		
		double	height_above_terrian_;		///< 视点在地表上的高度
		short	terrain_elevation_;			///<
		short	cur_terrain_elevation_;		///< 当前经纬度的地形高程		
		double	track_height_;				///< 		
		float   frustum_val[6][4];			///<
		double  min_tilt_;					///< _tilt可取的最小值
		double  max_tilt_;					///< _tilt可取的最大值
		double  maximum_altitude_;			///< 视点高度可取的最大值
		double  view_range;					///<	
		double  true_view_range;			///<	
		int     last_step_zoom_tick_count;	///<
		vec3d   camera_up_vector;			///< 摄像机up向量
		vec3d	position_;					///< 视点在全局坐标系下的位置
		vec3d	reference_center_;			///< 视线与地球交点，也是局部化中心点			
		Ellipsoid* earth_;					///< 封装地球模型信息		
		bool	underground_;				///< bool型变量标记是否能进入地下，true表可进入地下
		// Camera Reset variables
		static int last_reset_time;			///< Used by Reset() to keep track type of reset.
		const int  kDoubleTapDelay;			///< Double tap max time (ms)= 3000; 
		float cur_camera_elevation_;		///< 当前视点高度
		float target_camera_elevation_;		///< 目标视点高度
		// 以下用于LOCAL模式
		CoordinateSystem csys_;				///< 标记当前坐标系模式GLOBAL或者LOCAL
		double local_cenlon_;				///< 局部坐标系原点的经度
		double local_cenlat_;				///< 局部坐标系原点的纬度
		vec3d local_up_;					///< 局部坐标系下摄像机up值
		vec3d local_dir_;					///< 局部坐标系下视线方向
		vec3d local_eye_;					///< 局部坐标系下视点位置
		vec3d local_target_;				///< 局部坐标系下视线方向一点
		vec3d local_origin_;				///< 局部坐标系原点在全局坐标系下的坐标
		mat4d local_local2world_;			///< LOCAL模式下局部坐标到全局坐标的转换矩阵
		mat4d local_world2local_;			///< LOCAL模式下全局坐标到局部坐标的转换矩阵
		float csys_change_height_;			///< 坐标模式切换的视点高程临界值
		// 以下用于非轨迹球
		vec3d target_;						///< 非轨迹球模式下视线方向上一点坐标
		vec3d up_;							///< 非轨迹球模式下摄像机up值
		bool  b_trackball_;					///< bool型变量标记是否处于轨迹球模式下,ture表轨迹球模式
		bool b_is_auto;
		
	private:				
		float perspective_near_plane_distance_;	///< 视锥远近平面的距离
		float perspective_far_plane_distance_;	///< 视锥远近平面的距离
		mat4f m_view_matrix_;				///< View matrix used in last render.
		mat4d m_world_matrix_;				///< = Matrix.Identity;
		mat4f m_absolute_projection_matrix_;	///< 投影矩阵
		mat4f m_absolute_view_matrix_;		///< 
		vec4i viewport_;					///< 存储摄像机viewport信息（lx，ly，wid，hei）
		vec3d rel_camera_pos_;				///< 
		vec3f view_dir_;					///< 		
		//世界坐标下的太阳方向，放在这里不是很合适，急用
		vec3d sundir_;						///< 太阳方向		
		vec3d curup_;						///< 当前向上方向向量
		vec3d curnorth_;					///< 当前向北方向向量
		vec3d cureast_;						///< 当前向东方向向量
		mat4f local2world_;					///< 局部坐标到世界坐标转换矩阵	
		mat4f world2local_;					///< 世界坐标到局部坐标转换矩阵	
		Screen scr;
	};
}

#endif
