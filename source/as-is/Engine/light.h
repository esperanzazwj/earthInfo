#pragma once
#include<string>
#include "Movable.h"
#include "mathlib.h"
#include "NameGenerator.h"
#include "PreDefine.h"
//#include "shadowMapHelper.h"

enum ShadowTech {
	NO_Shadow = 0,
	NativeShadowMap = 1,
	PCF = 2,
	PCSS = 3,
	CSM = 4,
	VSM = 5,
	TechNum
};

struct light_struct {
	float position[3];
	float maxdist;
	float intensity[3];
	float exponent;
	float direction[3];
	float cutoff;

	float shadowmat[16];
};
struct LightData
{
	Vector4 position;
	Vector3 intensities;
	float attenuation;
	Vector3 coneDirection;
	float coneAngle;

};
class Light : public Movable
{
public:
	enum LightType
	{
		LT_UNKNOWN = 0,
		LT_POINT = 1,
		LT_DIRECTIONAL = 2,
		LT_SPOT = 3
	};

	Light();
	Light(const string &name, LightType type, SceneManager * m_creator = NULL);
	/** default destructor
	*/
	virtual ~Light() {}

	LightType getLightType() const { return m_Type; }

	float    getMaxDistance() { return m_MaximumDistance; }
	void setPosition(Vector3 pos)
	{
		if (pos == Position)
			return;
		Position = pos;
		m_moved = true;
		m_camera_valid = false;

	}
	Vector3& getPosition()
	{
		return Position;
	}

	void setDirection(Vector3 Dir)
	{
		Dir.normalize();
		if (Dir == Direction)
			return;
		Direction = Dir;
		m_moved = true;
		m_camera_valid = false;


	}

	Vector3& getDirection()
	{
		return Direction;
	}

	/** update light direction to target
	*/
	void updateDirection()
	{
		setDirection(Target - Position);
	}

	/** set the distance
	*/
	void     SetMaxDistance(float dist) { m_MaximumDistance = dist; }
	float calcMaxdistance();
	void    SetType(LightType type) { m_Type = type; }

	/**check is this light turn o
	*/
	bool isLightOn()
	{
		return mState;
	}
	/** set light state
	*/
	void setLight(bool state)
	{
		mState = state;
	}
	/** check if this light cast shadow
	*/
	bool isCastShadow()
	{
		return mCastShadow;
	}
	/** set light cast shadow or not
	*/
	void setCastShadow(bool castshadow)
	{
		mCastShadow = castshadow;
	}
	void CalcScissorRect(Camera* camera, float r, Vector4& rect);

	Matrix4 getShadowMatrix();
	Camera* getCamera();
	void completeMove()
	{
		m_moved = false;
	}
	void CollectData(vector<Light*> lights, void*& p, int& structsize, int& number);
	void CollectData(void*& p, int& structsize);
	void CollectData(vector<Light*> lights, vector<float>& p, int& structsize, const string& name);
protected:
	/** action to be taken when this object is moved.
	*/
	virtual void moveActionImp()
	{
		m_moved = true;
	}
	virtual void getBoundingBoxImp()
	{
		m_boundingBox.setNull();
	}
	virtual void getBoundingSphereImp()
	{
		m_sphere.radius = 0;
	}
public:
	/** color intensity
	*/
	Vector4  Diffuse;
	Vector4  Specular;

	/** light frustum params
	*/
	float fov;
	float near;
	float far;
	float width;
	float height;
	float radius;
	float bias;
	int samplePattern = 0;
	bool perspective = true;
	bool manualNearFar = false;

	/** types of shadow
	*/
	ShadowTech tech;

	/** attenuation for point light and spot light
	*/
	float    ConstantAttenuation;
	float    LinearAttenuation;
	float    QuadraticAttenuation;

	/** position of the light ,it is valid for point light
	and spot light.
	*/

	float   Exponent;
	/** and angle used by spot light
	*/
	float   Cutoff;

	/** ShadowMap Matrix.only useful for directional and spotlight
	*/
	Matrix4 ShadowMatrix;

	/** ShadowMap Bias
	*/
	float ShadowBias;
	Camera* m_camera;
	bool m_camera_valid;
	bool m_shadowmatrix_valid;

private:
	/** type of this light
	*/
	LightType m_Type;
	/** maximum distance to be influenced by this light
	*/
	float   m_MaximumDistance;
	/** Light on or off
	*/
	bool mState;
	/** light cast shadow or not
	*/
	bool mCastShadow;

	Vector3  Position;
	/** direction of the light,it is valid for directional light
	and spot light.
	*/
	Vector3 Direction;
	/** target position the light is pointing to
	*/
	Vector3 Target;

	/** name generator
	*/
	static NameGenerator* m_NameGenerator;

	/** name
	*/
	string name;
};
