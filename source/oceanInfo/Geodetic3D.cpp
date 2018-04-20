/**
 * @file Geodetic3D.cpp
 *
 */

#include "Geodetic3D.h"
#include "Geodetic2D.h"

namespace VirtualGlobeCore
{
	Geodetic3D::Geodetic3D(const Geodetic2D& geodetic2D)
	{
		_longitude = geodetic2D.getLongitude();
		_latitude = geodetic2D.getLatitude();
		_height = 0;
	}
	Geodetic3D::Geodetic3D(const Geodetic2D& geodetic2D, double height)
	{
		_longitude = geodetic2D.getLongitude();
		_latitude = geodetic2D.getLatitude();
		_height = height;
	}
}
