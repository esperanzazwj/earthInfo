/** @file Ellipsoid.cpp
 *
 *  @author DingXin: add comment
 */
#include "Ellipsoid.h"
#include "../Geodetic2D.h"
#include "../math/mat4.h"

namespace VirtualGlobeCore
{
	const Ellipsoid Ellipsoid::Wgs84(6378137.0, 6378137.0, 6356752.314245);
	const Ellipsoid Ellipsoid::ScaledWgs84(1.0, 1.0, 6356752.314245/6378137.0);
	const Ellipsoid Ellipsoid::UnitSphere(1.0, 1.0, 1.0);
	int sign(double val)
	{
		if(val>0)
			return 1;
		else if(val ==0)
			return 0;
		else  return -1;
	}
	Ellipsoid::Ellipsoid(double x, double y, double z)
	{
		_radii.x = x;
		_radii.y = y;
		_radii.z = z;
		_radiiSquared = vec3d(
			x * x,
			y * y,
			z * z);
		_radiiToTheFourth = vec3d(
			_radiiSquared.x * _radiiSquared.x,
			_radiiSquared.y * _radiiSquared.y,
			_radiiSquared.z * _radiiSquared.z);
		_oneOverRadiiSquared = vec3d(
			1.0 / (x * x), 
			1.0 / (y * y), 
			1.0 / (z * z));
	}

	Ellipsoid::Ellipsoid(const vec3d& radii)
	{
		_radii = radii;
		_radiiSquared = vec3d(
			radii.x * radii.x,
			radii.y * radii.y,
			radii.z * radii.z);
		_radiiToTheFourth = vec3d(
			_radiiSquared.x * _radiiSquared.x,
			_radiiSquared.y * _radiiSquared.y,
			_radiiSquared.z * _radiiSquared.z);
		_oneOverRadiiSquared = vec3d(
			1.0 / (radii.x * radii.x), 
			1.0 / (radii.y * radii.y), 
			1.0 / (radii.z * radii.z));
	}
	vec3d Ellipsoid::CentricSurfaceNormal(const vec3d& positionOnEllipsoid)
	{
		return positionOnEllipsoid.normalize();
	}

	vec3d Ellipsoid::GeodeticSurfaceNormal(const vec3d& positionOnEllipsoid)
	{
		return (positionOnEllipsoid * _oneOverRadiiSquared).normalize();
	}
	vec3d Ellipsoid::GeodeticSurfaceNormal(const Geodetic3D& geodetic)
	{
		double cosLatitude = cos(geodetic.getLatitude());

		return vec3d(
			cosLatitude * cos(geodetic.getLongitude()),
			cosLatitude * sin(geodetic.getLongitude()),
			sin(geodetic.getLatitude()));
	}

	vec3d Ellipsoid::ToVector3D(const Geodetic3D& geodetic)
	{
		vec3d n = GeodeticSurfaceNormal(geodetic);
		vec3d k = _radiiSquared*n;
		double gamma = sqrt(
			(k.x * n.x) +
			(k.y * n.y) +
			(k.z * n.z));

		vec3d rSurface = k / gamma;
		return rSurface + ( n * geodetic.getHeight());
	}

	vec3d Ellipsoid::ScaleToGeodeticSurface(vec3d position)//这函数有时候会死循环
	{
		double beta = 1.0 / sqrt(
			(position.x * position.x) * _oneOverRadiiSquared.x +
			(position.y * position.y) * _oneOverRadiiSquared.y +
			(position.z * position.z) * _oneOverRadiiSquared.z);
		double n = vec3d(
			beta * position.x * _oneOverRadiiSquared.x,
			beta * position.y * _oneOverRadiiSquared.y,
			beta * position.z * _oneOverRadiiSquared.z).length();
		double alpha = (1.0 - beta) * (position.length() / n);

		double x2 = position.x * position.x;
		double y2 = position.y * position.y;
		double z2 = position.z * position.z;

		double da = 0.0;
		double db = 0.0;
		double dc = 0.0;

		double s = 0.0;
		double dSdA = 1.0;

		do
		{
			alpha -= (s / dSdA);

			da = 1.0 + (alpha * _oneOverRadiiSquared.x);
			db = 1.0 + (alpha * _oneOverRadiiSquared.y);
			dc = 1.0 + (alpha * _oneOverRadiiSquared.z);

			double da2 = da * da;
			double db2 = db * db;
			double dc2 = dc * dc;

			double da3 = da * da2;
			double db3 = db * db2;
			double dc3 = dc * dc2;

			s = x2 / (_radiiSquared.x * da2) +
				y2 / (_radiiSquared.y * db2) +
				z2 / (_radiiSquared.z * dc2) - 1.0;

			dSdA = -2.0 *
				(x2 / (_radiiToTheFourth.x * da3) +
				y2 / (_radiiToTheFourth.y * db3) +
				z2 / (_radiiToTheFourth.z * dc3));
		}
		while (abs(s) > 1e-10);

		return vec3d(
			position.x / da,
			position.y / db,
			position.z / dc);
	}

	Geodetic2D Ellipsoid::ToGeodetic2D(vec3d positionOnEllipsoid)
	{
		vec3d n = GeodeticSurfaceNormal(positionOnEllipsoid);
		return Geodetic2D(atan2(n.y, n.x),
			asin(n.z / n.length()));
	}

	Geodetic3D Ellipsoid::ToGeodetic3D(const vec3d& position)
	{
		vec3d p = ScaleToGeodeticSurface(position);
		vec3d h = position - p;
		double height = sign(h.dotproduct(position)) * h.length();
		return Geodetic3D(ToGeodetic2D(p), height);
	}
	vec3d Ellipsoid::WorldDirToLocalDir(const Geodetic3D& geodetic, const vec3d& worldDir)
	{
		double longitude = geodetic.getLongitude();
		double latitude = geodetic.getLatitude();

		// Compute up vector
		vec3d    up      ( cos(longitude)*cos(latitude), sin(longitude)*cos(latitude), sin(latitude));

		// Compute east vector
		vec3d    east    (-sin(longitude), cos(longitude), 0);
		// Compute north vector = outer product up x east
		vec3d    north   = up.crossProduct(east);

		//vec3d pos = ToVector3D(geodetic);
		mat4d localtoWorld = mat4d::translate(vec3d(0,0,0));

		// set matrix
		double m[4][4];
		memcpy(m, localtoWorld.coefficients(), sizeof(double)*16);
		m[0][0] = east[0];
		m[1][0] = east[1];
		m[2][0] = east[2];

		m[0][1] = north[0];
		m[1][1] = north[1];
		m[2][1] = north[2];

		m[0][2] = up[0];
		m[1][2] = up[1];
		m[2][2] = up[2];

		localtoWorld = mat4d(m);
		mat4d worldTolocal = localtoWorld.inverse();
		vec3d localDir = worldTolocal * vec3d(worldDir);
		localDir = localDir.normalize();
		return localDir;
	}

	vec3d Ellipsoid::LocalDirToWorldDir(const Geodetic3D& geodetic, const vec3d& LocalDir)
	{
		double longitude = geodetic.getLongitude();
		double latitude = geodetic.getLatitude();

		// Compute up vector
		vec3d    up      ( cos(longitude)*cos(latitude), sin(longitude)*cos(latitude), sin(latitude));

		// Compute east vector
		vec3d    east    (-sin(longitude), cos(longitude), 0);
		// Compute north vector = outer product up x east
		vec3d    north   = up.crossProduct(east);

		//vec3d pos = ToVector3D(geodetic);
		mat4d localtoWorld = mat4d::translate(vec3d(0,0,0));

		// set matrix
		double m[4][4];
		memcpy(m, localtoWorld.coefficients(), sizeof(double)*16);
		m[0][0] = east[0];
		m[1][0] = east[1];
		m[2][0] = east[2];

		m[0][1] = north[0];
		m[1][1] = north[1];
		m[2][1] = north[2];

		m[0][2] = up[0];
		m[1][2] = up[1];
		m[2][2] = up[2];

		localtoWorld = mat4d(m);
		vec3d worldDir = localtoWorld * vec3d(LocalDir);
		worldDir = worldDir.normalize();
		return worldDir;
	}

	void Ellipsoid::ComputeCurve(const vec3d& start, const vec3d& stop, double granularity, std::vector<vec3d>& posArray)
	{
		vec3d normal = start.crossProduct(stop).normalize();
		double theta = start.angleBetween(stop);
		int n = max((int)(theta / granularity) - 1, 0);

		posArray.push_back(start);

		for (int i = 1; i <= n; ++i)
		{
			double phi = (i * granularity);

			posArray.push_back(start.rotateAroundAxis(normal, phi));
		}

		posArray.push_back(stop);
	}

	void Ellipsoid::ComputeRing(const vec3d& start, const vec3d& towards, double granularity, std::vector<vec3d>& posArray)
	{
		vec3d normal = start.crossProduct(towards).normalize();
		double theta = 2* M_PI;
		int n = max((int)(theta / granularity) - 1, 0);

		posArray.push_back(start);

		for (int i = 1; i <= n; ++i)
		{
			double phi = (i * granularity);

			posArray.push_back(start.rotateAroundAxis(normal, phi));
		}

		posArray.push_back(start);
	}

	void Ellipsoid::LonLatMove( double curLon, double curLat,
		double x, double y,  double &dstLon,double &dstLat)
	{
		double r = _radii.x;
		double curLatR = r * cos(curLat);
		double deltaX = curLatR;
		double deltaY = r;
		deltaX = x/deltaX;
		deltaY = y/deltaY;
		dstLat = curLat + deltaY;
		dstLon = curLon + deltaX;
	}
}
