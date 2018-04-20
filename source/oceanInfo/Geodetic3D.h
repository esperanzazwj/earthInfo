/** @file Geodetic3D.h
 *
 *  @author DingXin: add comment
 */

#ifndef _VGC_GEODETIC3D_H_
#define _VGC_GEODETIC3D_H_

#include "math/vec3.h"

/**
 * @attention geodetic类都是弧度制的
 */
namespace VirtualGlobeCore
{
    class Geodetic2D;

    /**
     * @class  Geodetic3D
     * @brief 三维地理位置
     * @details 每个位置包含经纬度和高度信息，以及判断是否为同一位置的函数
     */
	class Geodetic3D
	{
	public:
		Geodetic3D()
		{}
		Geodetic3D(double longitude, double latitude, double height)
		{
			_longitude = longitude;
			_latitude = latitude;
			_height = height;
		}
		Geodetic3D(const Geodetic2D& geodetic2D) ;
		Geodetic3D(const Geodetic2D& geodetic2D, double height);
		double getLongitude() const
		{
			return _longitude;
		}

		double getLatitude() const
		{
			return _latitude;
		}

		double getHeight() const
		{
			return _height;
		}

		bool Equals(const Geodetic3D& other)
		{
			return _longitude == other._longitude && _latitude == other._latitude && _height == other._height;
		}

		double _longitude;
		double _latitude;
		double _height;
	};
}

#endif
