/**
 * @file Geodetic2D.cpp
 *
 */

#include "Geodetic2D.h"
#include "Geodetic3D.h"

namespace VirtualGlobeCore
{
	Geodetic2D::Geodetic2D(const Geodetic3D& geodetic3D)
	{
		_longitude = geodetic3D.getLongitude();
		_latitude = geodetic3D.getLatitude();
	}

	double Geodetic2D::getLongitude() const 
	{
		return _longitude;
	}
}