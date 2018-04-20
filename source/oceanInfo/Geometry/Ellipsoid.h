/** @file Ellipsoid.h
 *  @author skillzero
 */

#ifndef _VGC_ELLIPSOID_H_
#define _VGC_ELLIPSOID_H_

#include <vector>

#include "../math/vec3.h"
#include "../Geodetic3D.h"

using namespace VirtualGlobeRender;

#define dmax(a,b)    (((a) > (b)) ? (a) : (b))
#define dmin(a,b)    (((a) < (b)) ? (a) : (b))

namespace VirtualGlobeCore
{
    /**
     *  @class Ellipsoid
	 *  @brief 椭球体的数学模型，一般用于地球上的坐标和方向运算
     */
	class Ellipsoid
	{
	public:
		static const Ellipsoid Wgs84;///< wgs84标准的地球模型
		static const Ellipsoid ScaledWgs84;///< 归一化的wgs84标准的地球模型
		static const Ellipsoid UnitSphere;///< 轴半径都是1的纯圆球体模型

		/**
		*  @brief 构造函数
		*  @param x x轴长度 
		*  @param y y轴长度
		*  @param z z轴长度
	    */
		Ellipsoid(double x, double y, double z);

		/**
		*  @brief 构造函数
		*  @param radii 三轴长度
	    */
		Ellipsoid(const vec3d& radii);

		/**
		*  @brief 获得地球坐标系下的地球点上，该点的法向
		*  @param positionOnEllipsoid 求取点的坐标
		*  @return 法向
	    */
		vec3d CentricSurfaceNormal(const vec3d& positionOnEllipsoid);

		/**
		*  @brief 获得地球坐标系下的地球点上，该点的法向
		*  @param positionOnEllipsoid 求取点的坐标
		*  @return 法向
	    */
		vec3d GeodeticSurfaceNormal(const vec3d& positionOnEllipsoid);

		/**
		*  @brief 获得地球坐标系下的地球点上，该点的法向
		*  @param geodetic 求取点的坐标
		*  @return 法向
	    */
		vec3d GeodeticSurfaceNormal(const Geodetic3D& geodetic);

		/**
		*  @brief 获得地球坐标系下的地球点经纬高坐标向地球坐标的转换
		*  @param geodetic 求取点的经纬高坐标
		*  @return 输入点的地球坐标
	    */
		vec3d ToVector3D(const Geodetic3D& geodetic);

		vec3d _radii;///< 三轴长度
		vec3d _radiiSquared;///< 三轴平方
		vec3d _oneOverRadiiSquared;///< 三轴平方做分母
		vec3d _radiiToTheFourth;
		double MaximumRadius()
		{
			return dmax(_radii.x, dmax(_radii.y, _radii.z)); 
		}
		double MinimumRadius()
		{
			return dmin(_radii.x, dmin(_radii.y, _radii.z)); 
	     }
		vec3d ScaleToGeodeticSurface(vec3d position);
		/**
		*  @brief 获得地球坐标系下的地球坐标点向经纬高坐标的转换
		*  @param positionOnEllipsoid 地球点
		*  @return 输入点的经纬坐标
	    */
		Geodetic2D ToGeodetic2D(vec3d positionOnEllipsoid);
		/**
		*  @brief 获得地球坐标系下的地球坐标点向经纬高坐标的转换
		*  @param position 地球点
		*  @return 输入点的经纬高坐标
	    */
		Geodetic3D ToGeodetic3D(const vec3d& position);
		/**
		*  @brief 获得地球坐标系下的某点从世界坐标向局部坐标系下的方向转换
		*  @param worldDir 世界方向
		*  @return 输入方向的局部方向
	    */
		vec3d WorldDirToLocalDir(const Geodetic3D& geodetic, const vec3d& worldDir);
		/**
		*  @brief 获得地球坐标系下的局部方向向世界方向的转换
		*  @param LocalDir 局部方向
		*  @return 输入方向在该经纬度下的世界方向
	    */
		vec3d LocalDirToWorldDir(const Geodetic3D& geodetic, const vec3d& LocalDir);

		void  ComputeCurve(const vec3d& start, const vec3d& stop, double granularity, std::vector<vec3d>& posArray);
		void  ComputeRing(const vec3d& start, const vec3d& towards, double granularity, std::vector<vec3d>& posArray);

        
        /**
		 * @brief 在地球上一个输入的经纬度上通过移动距离，得到的目标经纬度
	     * @param curLon 当前经度，弧度
         * @param curLat 当前纬度，弧度
         * @param x 移动距离，单位米，负值向西，正值向东
         * @param y 移动距离，单位米，负值向南，正值向北
         * @param dstLon 目的地经度
         * @param dstLat 目的地纬度
	     */
		void LonLatMove( double curLon, double curLat,
			double x, double y,  double &dstLon,double &dstLat);
	protected:
		
	};
	
}
#endif
