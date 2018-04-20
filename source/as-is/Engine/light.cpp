#include "light.h"
#include "SceneNode.h"
#include "Camera.h"
#include "../../util/algorithm.hpp"
#include <algorithm>

// static members
NameGenerator* Light::m_NameGenerator = new NameGenerator("Light");

Light::Light(const string &name, LightType type, SceneManager * mcreator)
	: Movable(name, MT_LIGHT, mcreator),
	m_Type(type),
	m_MaximumDistance(INFINITY)
{
	Diffuse = Vector4{Vector3::UNIT_SCALE, 1.0f};
	Specular = Vector4{Vector3::ZERO, 1.0f};
	ConstantAttenuation = 1.0f;
	LinearAttenuation = 0.0f;
	QuadraticAttenuation = 0.0f;
	Position = Vector3::ZERO;
	Direction = Vector3::NEGATIVE_UNIT_Z;
	Exponent = 60.0f;
	Cutoff = 60.0f;
	mCastShadow = false;
	mState = true;
	ShadowBias = 0.0005f;
	m_camera = mcreator->CreateCamera(name + "_camera");

	m_camera_valid = false;
	m_shadowmatrix_valid = false;

}

Light::Light() :Movable(), m_Type(LT_UNKNOWN)
{
	m_Name = m_NameGenerator->Generate();
	Diffuse = Vector4{Vector3::UNIT_SCALE, 1.0f};
	Specular = Vector4{Vector3::ZERO, 1.0f};
	ConstantAttenuation = 1.0f;
	LinearAttenuation = 0.0f;
	QuadraticAttenuation = 0.0f;
	Position = Vector3::ZERO;
	Target = Vector3::ZERO;
	Direction = Vector3::NEGATIVE_UNIT_Z;
	Exponent = 60.0f;
	Cutoff = constants::pi * 0.25f;
	mCastShadow = true;
	mState = true;
}

void Light::CollectData(vector<Light*> lights, void*& _p, int& totalsize, int& number)
{
	auto p = new light_struct[lights.size() + 1];

    _p = p;
	number = (int)lights.size();
	totalsize = sizeof(light_struct)*(number + 1);

	p->position[0] = (float)number;
    for (auto& l: lights) {
        p++;
		p->exponent = l->Exponent;
		p->cutoff = l->Cutoff;
		p->maxdist = l->m_MaximumDistance;
        ss::util::copy_bits(l->Position, p->position);
        ss::util::copy_bits(l->Diffuse, p->intensity);
        ss::util::copy_bits(l->Direction, p->direction);
        ss::util::copy_bits(l->getShadowMatrix(), p->shadowmat);
	}
}

void Light::CollectData(void*& _p, int& structsize)
{
	auto p = new light_struct;
    _p = p;
	structsize = sizeof(light_struct);

    p->exponent = Exponent;
    p->cutoff = Cutoff;
    p->maxdist = m_MaximumDistance;
    ss::util::copy_bits(Position, p->position);
    ss::util::copy_bits(Diffuse, p->intensity);
    ss::util::copy_bits(Direction, p->direction);
    ss::util::copy_bits(getShadowMatrix(), p->shadowmat);
}

void Light::CollectData(vector<Light*> lights, vector<float>& p, int& float_num, const string& name)
{
	if (name == "position") {
		float_num = 3 * (int)lights.size();
		for (int i = 0; i < (int)lights.size(); i++)
		{
			Light* l = lights[i];
			p.push_back(l->Position.x);
			p.push_back(l->Position.y);
			p.push_back(l->Position.z);

		}
		return;
	}
	if (name == "color") {
		float_num = 3 * (int)lights.size();
		for (int i = 0; i < (int)lights.size(); i++)
		{
			Light* l = lights[i];
			p.push_back(l->Diffuse.x);
			p.push_back(l->Diffuse.y);
			p.push_back(l->Diffuse.z);

		}
		return;
	}
	if (name == "direction") {
		float_num = 3 * (int)lights.size();
		for (int i = 0; i < (int)lights.size(); i++)
		{
			Light* l = lights[i];
			p.push_back(l->Direction.x);
			p.push_back(l->Direction.y);
			p.push_back(l->Direction.z);

		}
		return;
	}
	if (name == "other") {
		float_num = 3 * (int)lights.size();
		for (int i = 0; i < (int)lights.size(); i++)
		{
			Light* l = lights[i];
			p.push_back(l->Exponent);
			p.push_back(l->Cutoff);
			p.push_back(l->m_MaximumDistance);
		}
		return;
	}
}

HW::Matrix4 Light::getShadowMatrix()
{
	if (m_camera_valid == false || m_shadowmatrix_valid == false) {
        auto camera = getCamera();
        auto bias = Matrix4{
            Matrix3{0.5f, 0.5f, 0.5f},
            Vector3{0.5f, 0.5f, 0.5f},
        };

		ShadowMatrix = bias *camera->getProjectionMatrixDXRH() * camera->getViewMatrix();
		m_shadowmatrix_valid = true;
	}
	return ShadowMatrix;
}

Camera* Light::getCamera()
{
	if (m_camera_valid == false) {
		m_camera->lookAt(Position, Position + Direction, Vector3(0, 0, 1));
		float fov = std::min(90.0f, 2.2f * std::acos(Cutoff) * 180.0f / constants::pi);
		m_camera->setPerspective(fov, 1.0, 1.0, 1000);
		m_camera_valid = true;
	}
	return m_camera;
}

float Light::calcMaxdistance()
{
	return m_MaximumDistance;
}

void Light::CalcScissorRect(Camera* camera, float r, Vector4& rect)
{
	// Create a bounding sphere for the light., based on the position
	// and range
	Matrix4 ViewMatrix = camera->getViewMatrix();
	Matrix4 ProjMatrix = camera->getProjectionMatrixDXRH();
	Vector4 centerWS = Vector4(Position.x, Position.y, Position.z, 1.0f);
	float radius = r;
	// Transform the sphere center to view space
	Vector4 centerVS = ViewMatrix * centerWS;
	// Figure out the four points at the top, bottom, left, and
	// right of the sphere
	Vector4 topVS = centerVS + Vector4(0.0f, radius, 0.0f, 0.0f);
	Vector4 bottomVS = centerVS - Vector4(0.0f, radius, 0.0f, 0.0f);
	Vector4 leftVS = centerVS - Vector4(radius, 0.0f, 0.0f, 0.0f);
	Vector4 rightVS = centerVS + Vector4(radius, 0.0f, 0.0f, 0.0f);
	// Figure out whether we want to use the top and right from quad
	// tangent to the front of the sphere, or the back of the sphere
	leftVS.z = leftVS.x < 0.0f ? leftVS.z + radius : leftVS.z - radius;
	rightVS.z = rightVS.x < 0.0f ? rightVS.z - radius : rightVS.z + radius;
	topVS.z = topVS.y < 0.0f ? topVS.z - radius : topVS.z + radius;
	bottomVS.z = bottomVS.y < 0.0f ? bottomVS.z + radius : bottomVS.z - radius;

	float m_fFarClip = -camera->getNear();
	float m_fNearClip = -camera->getFar();
	// Clamp the z coordinate to the clip planes
	leftVS.z = Clamp(leftVS.z, m_fNearClip, m_fFarClip);
	rightVS.z = Clamp(rightVS.z, m_fNearClip, m_fFarClip);
	topVS.z = Clamp(topVS.z, m_fNearClip, m_fFarClip);
	bottomVS.z = Clamp(bottomVS.z, m_fNearClip, m_fFarClip);
	// Figure out the rectangle in clip-space by applying the
	// perspective transform. We assume that the perspective
	// transform is symmetrical with respect to X and Y.
	float rectLeftCS = -leftVS.x * ProjMatrix[0][0] / leftVS.z;
	float rectRightCS = -rightVS.x * ProjMatrix[0][0] / rightVS.z;
	float rectTopCS = -topVS.y * ProjMatrix[1][1] / topVS.z;
	float rectBottomCS = -bottomVS.y * ProjMatrix[1][1] / bottomVS.z;

	//Vector4 ql = ProjMatrix*leftVS, qr = ProjMatrix*rightVS, qt = ProjMatrix*topVS,qb=ProjMatrix*bottomVS;
	//float rectLeftCS = ql.x/ql.w;
	//float rectRightCS = qr.x/qr.w;
	//float rectTopCS = qt.y/qt.w;
	//float rectBottomCS = qb.y/qb.w;
	// Clamp the rectangle to the screen extents
	rectTopCS = Clamp(rectTopCS, -1.0f, 1.0f);
	rectBottomCS = Clamp(rectBottomCS, -1.0f, 1.0f);
	rectLeftCS = Clamp(rectLeftCS, -1.0f, 1.0f);
	rectRightCS = Clamp(rectRightCS, -1.0f, 1.0f);

	rect = Vector4(rectLeftCS, rectBottomCS, rectRightCS, rectTopCS);
	// Now we convert to screen coordinates by applying the

}

