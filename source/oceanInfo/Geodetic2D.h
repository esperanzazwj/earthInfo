/** @file Geodetic2D.h
 *
 *  @author DingXin: add comment
 */

#ifndef _VGC_GEODETIC2D_H_
#define _VGC_GEODETIC2D_H_
#include "math/vec3.h"

namespace VirtualGlobeCore
{
	class Geodetic3D;

    /**
     * @class  Geodetic2D
     * @brief 二维地理位置
     * @details 每个位置包含经纬度信息，以及判断是否为同一位置的函数。
     * 可以从三维信息中构造二维信息。
     */
	class Geodetic2D
	{
	public:
		Geodetic2D(double longitude, double latitude)
		   {
			   _longitude = longitude;
			   _latitude = latitude;
		   }
		Geodetic2D(const Geodetic3D& geodetic3D);
		double getLongitude() const ;
		double getLatitude() const
		{
			return _latitude;
		}

		bool EqualsEpsilon(const Geodetic2D& other, double epsilon)
		{
                  return (std::abs(_longitude - other._longitude) <= epsilon) &&
                      (std::abs(_latitude - other._latitude) <= epsilon);
		}

		bool Equals(const Geodetic2D& other)
		{
			return _longitude == other._longitude && _latitude == other._latitude;
		}
		 
		protected:
		
		 double _latitude;
		 double _longitude;
	};
}

#endif
