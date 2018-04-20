#ifndef _QUAT_H_
#define _QUAT_H_

#include "pmath.h"

#include "vec3.h"
#include "mat3.h"
#include "mat4.h"

namespace VirtualGlobeRender
{

/**
 * A templated quaternion class.
 * Important : assumes T is typeing point type.
 * @ingroup math
 */
template <typename type>
class quat {
public:
    /**
     * x coordinate.
     */
    type x;

    /**
     * y coordinate.
     */
    type y;

    /**
     * z coordinate.
     */
    type z;

    /**
     * w coordinate.
     */
    type w;

    /**
     * Creates a new, uninitialized quaternion.
     */
    quat();

    /**
     * Creates a new quaternion with the given coordinates (useless in general outside this header).
     */
    quat(type x, type y, type z, type w);

    /**
     * Creates a new quaternion with the given coordinates.
     */
    quat(const type v[4]);

    /**
     * Creates a new quaternion as a copy of the given quaternion.
     */
    quat(const quat& v);

    /**
     * Create a normalized quaternion from an axis and an angle (MOST USEFUL)
     * @param axis an axis (do not need to be normalized)
     * @param angle angle in radian
     */
    quat(const vec3<type>& axis, type angle);

    /**
     * Create a normalized quaternion which transform a vector into another.
     * No need to pass normalized vectors.
     */
    quat(const vec3<type>& to, const vec3<type>& from);

	quat<type> EulerToQuaternion(type yaw, type pitch, type roll);
	vec3<type> QuaternionToEuler();

    /**
     * Multiply by another quaternion (replace a 3x3 rotation matrix multiplication).
     */
    quat operator*(const quat &a) const;

    /**
     * Multiply directly by a vector. Returns a rotated vector.
     */
    vec3<type> operator*(const vec3<type>& a) const;

    /**
     * Multiply by another quaternion (replace a 3x3 rotation matrix multiplication).
     */
    quat& operator*=(const quat &a );

    /**
     * Returns the inverse of this quaternion.
     */
    quat inverse() const;

    /**
     * If length is 1, this is a normalized quaternion (the interesting ones).
     */
    type length() const;

    /**
     * Returns this quaternion normalized to unit length.
     */
    quat normalize() const;

    /**
     * Returns the correponding 3x3 rotation matrix.
     */
    mat3<type> toMat3() const;

    /**
     * Returns the correponding 4x4 rotation matrix.
     */
    mat4<type> toMat4() const;
	//sz add 12-6-4
	type dot(const quat &a) const;
	quat Slerp(const quat &q, double t) const;
    static const quat ONE;
};

/**
 * Quaternion using float.
 * @ingroup math
 */
typedef quat<float> quatf;

/**
 * Quaternion using double.
 * @ingroup math
 */
typedef quat<double> quatd;

// no, there is not quat<int>. Get over it.

// constructors

template <typename type>
inline quat<type>::quat( )
{
}

template <typename type>
inline quat<type>::quat(type xi, type yi, type zi, type wi) : x(xi), y(yi), z(zi), w(wi)
{
}

template <typename type>
inline quat<type>::quat(const type v[4]) : x(v[0]), y(v[1]), z(v[2]), w(v[3])
{
}

template <typename type>
inline quat<type>::quat(const quat& v) : x(v.x), y(v.y), z(v.z), w(v.w)
{
}

template <typename type>
inline quat<type>::quat(const vec3<type>& axis, type angle)
{
    vec3<type> axisN = axis.normalize();
    type a = angle * 0.5f;
    type sina = sin(a);
    type cosa = cos(a);
    x = axisN.x * sina;
    y = axisN.y * sina;
    z = axisN.z * sina;
    w = cosa;
}

template <typename type>
quat<type>::quat(const vec3<type>& to, const vec3<type>& from)
{
    vec3<type> f = from.normalize();
    vec3<type> t = to.normalize();

    type dotProdPlus1 = 1.0 + f.dotproduct(t);

    if (dotProdPlus1 < (type)1e-7) {
        w = 0;
        if (fabs(f.x) < 0.6) {
            double norm = sqrt(1 - f.x * f.x);
            x = 0;
            y = f.z / norm;
            z = -f.y / norm;
        } else if (fabs(f.y) < 0.6) {
            double norm = sqrt(1 - f.y * f.y);
            x = -f.z / norm;
            y = 0;
            z = f.x / norm;
        } else {
            double norm = sqrt(1 - f.z * f.z);
            x = f.y / norm;
            y = -f.x / norm;
            z = 0;
        }
    } else {
        type s = (type)sqrt(0.5 * dotProdPlus1);
        vec3<type> tmp = (f.crossProduct(t)) / (2.0 * s);
        x = tmp.x;
        y = tmp.y;
        z = tmp.z;
        w = s;
    }
}

//sz add 2012-6-1

template <typename type>
quat<type> quat<type>::EulerToQuaternion(type yaw, type pitch, type roll)
{
	type cy = cos(yaw * 0.5);
	type cp = cos(pitch * 0.5);
	type cr = cos(roll * 0.5);
	type sy = sin(yaw * 0.5);
	type sp = sin(pitch * 0.5);
	type sr = sin(roll * 0.5);

	w = cy*cp*cr + sy*sp*sr;
	x = sy*cp*cr - cy*sp*sr;
	y = cy*sp*cr + sy*cp*sr;
	z = cy*cp*sr - sy*sp*cr;

	return quat<type>(x, y, z, w);
}
template <typename type>
vec3<type> quat<type>::QuaternionToEuler()
{
	type q0 = w;
	type q1 = x;
	type q2 = y;
	type q3 = z;

	type x = atan2( 2 * (q2*q3 + q0*q1), (q0*q0 - q1*q1 - q2*q2 + q3*q3));
	type y = asin( -2 * (q1*q3 - q0*q2));
	type z = atan2( 2 * (q1*q2 + q0*q3), (q0*q0 + q1*q1 - q2*q2 - q3*q3));
	return vec3<type>(x,y,z);
}


// operations

template <typename type>
quat<type> quat<type>::operator*( const quat &a ) const
{//四元素的乘法是不可交换的，这里感觉ork原版的似乎把这个乘法写错了，按worldwind下的四元素重写了
    /*return quat(a.w * x  + a.x * w  + a.y * z  - a.z * y,
                a.w * y  - a.x * z  + a.y * w  + a.z * x,
                a.w * z  + a.x * y  - a.y * x  + a.z * w,
                a.w * w  - a.x * x  - a.y * y  - a.z * z);*/
	return quat(w * a.x + x * a.w + y * a.z - z * a.y,
		w * a.y + y * a.w + z * a.x - x * a.z,
		w * a.z + z * a.w + x * a.y - y * a.x,
		w * a.w - x * a.x - y * a.y - z * a.z);
}

template <typename type>
quat<type>& quat<type>::operator*=( const quat &a )
{
    *this = *this * a;
    return *this;
}

template <typename type>
vec3<type> quat<type>::operator*(const vec3<type>& a) const
{
    // TODO implement faster, direct multiplication
    return toMat3() * a;
}

template <typename type>
quat<type> quat<type>::inverse() const
{
    return quat( -x, -y, -z, w );
}

template <typename type>
type quat<type>::length() const
{
    type len = x * x + y * y + z * z + w * w;
    return static_cast<type>(sqrt(len));
}

template <typename type>
quat<type> quat<type>::normalize() const
{
    type invLength = 1 / length();
    return quat(x * invLength, y * invLength, z * invLength, w * invLength);
}

template <typename type>
mat3<type> quat<type>::toMat3() const
{
    type xx = x * x,
         xy = x * y,
         xz = x * z,
         xw = x * w,
         yy = y * y,
         yz = y * z,
         yw = y * w,
         zz = z * z,
         zw = z * w;
         return mat3<type>
         (
             1 - 2 * (yy + zz)    , 2 * (xy - zw)        , 2 * (xz + yw)    ,
             2 * (xy + zw)        , 1 - 2 * (xx + zz)    , 2 * (yz - xw)    ,
             2 * (xz - yw)        , 2 * (yz + xw)     , 1 - 2 * (xx + yy)
         );
 }

template <typename type>
mat4<type> quat<type>::toMat4() const
{
    return mat4<type>(toMat3());
}

template <typename type>
type quat<type>::dot(const quat &a) const
{
	return x*a.x + y*a.y+ z*a.z+ w*a.w;
}

template <typename type>
quat<type> quat<type>::Slerp(const quat &q, double t) const
{
	type cosom = x * q.x + y * q.y + z * q.z + w * q.w;
	type tmp0, tmp1, tmp2, tmp3;
	if (cosom < 0.0)
	{
		cosom = -cosom;
		tmp0 = -q.x;
		tmp1 = -q.y;
		tmp2 = -q.z;
		tmp3 = -q.w;
	}
	else
	{
		tmp0 = q.x;
		tmp1 = q.y;
		tmp2 = q.z;
		tmp3 = q.w;
	}

	/* calc coeffs */
	type scale0, scale1;

	if ((1.0 - cosom) > 4.94066e-324)
	{
		// standard case (slerp)
		type omega =  acos (cosom);
		type sinom = sin (omega);
		scale0 =  sin ((1.0 - t) * omega) / sinom;
		scale1 =  sin (t * omega) / sinom;
	}
	else
	{
		/* just lerp */
		scale0 = 1.0 - t;
		scale1 = t;
	}

	quat<type> newq;
	newq.x = scale0 * x + scale1 * tmp0;
	newq.y = scale0 * y + scale1 * tmp1;
	newq.z = scale0 * z + scale1 * tmp2;
	newq.w = scale0 * w + scale1 * tmp3;

	return newq;
}

template <typename type>
const quat<type> quat<type>::ONE(0, 0, 0, 1);

template <typename type>
quat<type> slerp(const quat<type>& from, const quat<type>& to, type t)
{
    if (t <= 0) {
        return quat<type>(from);
    } else if (t >= 1) {
        return quat<type>(to);
    } else {
        type cosom = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;
        type absCosom = fabs(cosom);

        type scale0;
        type scale1;

        if ((1 - absCosom) > 1e-6f) {
            type omega = safe_acos( absCosom );
            type sinom = 1 / sin( omega );
            scale0 = sin( ( 1 - t ) * omega ) * sinom;
            scale1 = sin( t * omega ) * sinom;
        } else {
            scale0 = 1 - t;
            scale1 = t;
        }
        quat<type> res = quat<type>(scale0 * from.x + scale1 * to.x,
                                    scale0 * from.y + scale1 * to.y,
                                    scale0 * from.z + scale1 * to.z,
                                    scale0 * from.w + scale1 * to.w);
        return res.normalize();
    }
}

}

#endif // _QUAT_H_
