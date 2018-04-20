#include "Camera.h"
#include "SceneNode.h"
#include "Movable.h"
#include "Entity.h"
#include "SubMesh.h"
#include "OMesh.h"
#include "ElemIterator.h"
#include <cmath>
#include "../../util/unreachable.macro.hpp"

//#include "Timer.h"

namespace HW
{
	Camera::Camera(const string& name,SceneManager* creator)
		: Movable(name,MT_CAMERA,creator),
		m_derivedPosValid(false),
		m_localCoordValid(false),
		m_viewMatrixValid(false),
		m_projMatrixValid(false),
		m_clipPlaneValid(false),
		m_renderQueueValid(false),
		m_ProjType(PT_PERSPECTIVE),
		target_to_use(NULL),
		renderable_test(false)
	{
		/// allocate memory for render queue in advance.
		intersect_num =0;
	}

	Camera::~Camera()
	{
		if(!m_RenderQueue.empty())
			m_RenderQueue.clear();
	}

	void Camera::setProjectionType(ProjectionType type)
	{

		if (m_ProjType != type)
		{
			m_ProjType = type;
			m_clipPlaneValid = false;
			m_projMatrixValid = false;
			m_renderQueueValid = false;
		}
	}

	void Camera::setPosition(const Vector3 &pos)
	{
		if (pos == m_Position)
			return;
		m_Position = pos;
		// invalid matrices.
		m_viewMatrixValid = false;
		m_derivedPosValid = false;
		m_clipPlaneValid = false;
		m_renderQueueValid = false;
	}

	void Camera::calcLocalSystem()
	{
		//m_Up = Vector3(0, 1, 0);
		//Vector3 mRight = m_Direction.crossProduct(m_Up);
		//m_Up = mRight.crossProduct(m_Direction);
		//m_Up.normalize();

	}
	/// return the position of the camera.
	/// if this camera is not attached to some scene node
	/// then the position is default in global space.
	Vector3 Camera::getPosition()
	{
		if( m_derivedPosValid)
			return m_DerivedPosition;
		/// parent exists
		if(m_parent)
		{
			m_DerivedPosition = m_parent->getOrientation()*m_Position + m_parent->getTranslation();
		}
		else
		{
			m_DerivedPosition = m_Position;
		}
		m_derivedPosValid = true;
		return m_DerivedPosition;
	}

	void Camera::setDirection(const Vector3 &dir)
	{
		Vector3 n_dir = dir;
		n_dir.normalize();
		if (m_Direction == n_dir)
			return;
		m_Direction = n_dir;
		//m_Direction.normalize();
		m_viewMatrixValid = false;
		/// invalidate local coordinate system.
		m_localCoordValid = false;
		m_clipPlaneValid = false;
		m_renderQueueValid = false;
	}

	Vector3 Camera::getDirection()
	{

		if(m_localCoordValid)
			return m_DerivedDirection;
		calcLocalSystem();
		if(m_parent)
		{
			m_DerivedDirection = m_parent->getOrientation()*m_Direction;
			m_DerivedUp = m_parent->getOrientation()*m_Up;
		}
		else
		{
			m_DerivedDirection = m_Direction;
			m_DerivedUp = m_Up;
		}
		m_localCoordValid = true;
		return m_DerivedDirection;
	}

	void Camera::lookAt(const Vector3 &eye, const Vector3 &focus, const Vector3 &up)
	{

		Vector3 dir = focus - eye;
		dir.normalize();
		if (m_Position == eye && dir == m_Direction && up == m_Up)
			return;
		m_Position = eye;
		//m_Direction = (focus - eye).hat();

		m_Direction = dir;

		m_Up = up;
		// invalidate view matrix point and local system
		m_derivedPosValid = false;
		m_viewMatrixValid = false;
		m_localCoordValid = false;
		m_renderQueueValid = false;
	}

	void Camera::lookAtBB(const Vector3 &eye, const BoundingBox& bb, const Vector3 &up)
	{
		Vector3 center = bb.getCenter();
		lookAt(eye, center, up);
		float fovy = 45.0f;
		setPerspective(fovy, 1.0f, 0.01f, 100.0f);
	}

	void Camera::setUp(const Vector3 &up)
	{
		if (m_Up == up)
			return;
		m_Up = up; // need not to be normalized.
		m_viewMatrixValid = false;
		m_localCoordValid = false;
		m_clipPlaneValid = false;
	}

	/// direction must already be set.
	Vector3 Camera::getUp()
	{
		if(m_localCoordValid)
			return m_DerivedUp;
		calcLocalSystem();
		if(m_parent)
		{
			m_DerivedDirection = m_parent->getOrientation()*m_Direction;
			m_DerivedUp = m_parent->getOrientation()*m_Up;
		}
		else
		{
			m_DerivedDirection = m_Direction;
			m_DerivedUp = m_Up;
		}
		m_localCoordValid = true;
		return m_DerivedUp;
	}

	void Camera::setFrustum(float left, float right, float bottom, float top, float tnear, float tfar)
	{
		//assert(left != right && bottom != top && tnear > 0.0 && tfar > tnear);
		if (left == m_Left && right == m_Right && bottom == m_Bottom && top == m_Top && tnear == m_Near && tfar == m_Far)
			return;
		m_Left = left;
		m_Right = right;
		m_Bottom = bottom;
		m_Top = top;
		m_Near = tnear;
		m_Far = tfar;

		m_projMatrixValid = false;
		m_clipPlaneValid = false;
		m_renderQueueValid = false;
	}

	void Camera::setPerspective(float fovy, float spec, float tnear, float tfar)
	{

		assert(fovy > 0.0f && spec > 0.0f && tnear > 0.0f && tfar > tnear);
		m_fov = fovy;
		m_Near = tnear;
		m_Far = tfar;
		float half_angle = fovy * constants::pi / 360.0f;
		m_Bottom = -m_Near*tan(half_angle);
		m_Top = -m_Bottom;
		m_Left = m_Bottom*spec;
		m_Right = -m_Left;
		m_projMatrixValid = false;
		m_clipPlaneValid = false;
		m_renderQueueValid = false;
		m_spec = spec;
	}

	void Camera::setProjectionMatrix(Matrix4 proj)
	{
		this->m_projMatrix = proj;
	}

	void Camera::setViewMatrix(Matrix4 view)
	{
		this->m_viewMatrix = view;
	}

	float Camera::getFov()
	{
		return m_fov;
	}

	float Camera::getNear()
	{
		return m_Near;
	}

	float Camera::getFar()
	{
		return m_Far;
	}

	float Camera::getLeft()
	{
		return m_Left;
	}

	float Camera::getRight()
	{
		return m_Right;
	}

	float Camera::getBottom()
	{
		return m_Bottom;
	}

	float Camera::getTop()
	{
		return m_Top;
	}

	float Camera::getAspect()
	{
		return m_spec;
	}


	void nvlookAt(Matrix4& M, Vector3& eye, Vector3& center, Vector3& up)
	{
		Vector3 x, y, z;

		// make rotation matrix

		// Z vector
		z.x = eye.x - center.x;
		z.y = eye.y - center.y;
		z.z = eye.z - center.z;
		z.normalize();

		// Y vector
		y.x = up.x;
		y.y = up.y;
		y.z = up.z;

		// X vector = Y cross Z
		x = y.crossProduct(z);

		// Recompute Y = Z cross X
		y = z.crossProduct(x);

		// cross product gives area of parallelogram, which is < 1.0 for
		// non-perpendicular unit-length vectors; so normalize x, y here
		x.normalize();
		y.normalize();

		M[0][0] = x.x; M[1][0] = x.y; M[2][0] = x.z; M[3][0] = -x.x * eye.x - x.y * eye.y - x.z*eye.z;
		M[0][1] = y.x; M[1][1] = y.y; M[2][1] = y.z; M[3][1] = -y.x * eye.x - y.y * eye.y - y.z*eye.z;
		M[0][2] = z.x; M[1][2] = z.y; M[2][2] = z.z; M[3][2] = -z.x * eye.x - z.y * eye.y - z.z*eye.z;
		M[0][3] = 0.0; M[1][3] = 0.0; M[2][3] = 0.0; M[3][3] = 1.0;
	}

	Matrix4 Camera::getViewMatrix()
	{
        if (m_viewMatrixValid) return m_viewMatrix;
		m_viewMatrixValid = true;

        auto z = -m_Direction;
        auto x = m_Up.crossProduct(z).hat();
        auto y = z.crossProduct(x).hat();
        auto inv_rot = Matrix3{x, y, z}.inverse_as_if_orthogonal();

        m_viewMatrix = Matrix4{
            inv_rot,
            inv_rot * -getPosition(),
        };

		return m_viewMatrix;
	}

	/// this function is based on the right handed coordinate system.
	/// clip space is {xy | x,y ranges -1~1 }{z | z ranges 0~1}
	Matrix4 Camera::calcPerspProjMatrixDXRH()
	{
		assert(m_Left != 0.0f && m_Right != 0.0 && m_Top != 0.0f && m_Bottom != 0.0f);
		assert(m_Near > 0.0f && m_Far > 0.0f);

        if (m_projMatrixValid) return m_projMatrix;
		m_projMatrixValid = true;

        m_projMatrix = Matrix4{
            Vector4{2.0f * m_Near / (m_Right-m_Left), 0.0f, 0.0f, 0.0f},
            Vector4{0.0f, 2.0f * m_Near / (m_Top-m_Bottom), 0.0f, 0.0f},
            Vector4{
                (m_Right+m_Left) / (m_Right-m_Left),
                (m_Top+m_Bottom) / (m_Top-m_Bottom),
                (m_Far+m_Near)   / (m_Near-m_Far),
                -1.0f,
            },
            Vector4{0.0f, 0.0f, 2.0f * m_Near * m_Far / (m_Near-m_Far), 0.0f},
        };
		return m_projMatrix;
	}

	Matrix4 Camera::calcOrthoProjMatrixOPENGL()
	{
		assert(m_Left != 0.0f && m_Right != 0.0 && m_Top != 0.0f && m_Bottom != 0.0f);
		assert(m_Near > 0.0f && m_Far > 0.0f);

        if (m_projMatrixValid) return m_projMatrix;
		m_projMatrixValid = true;

        auto w = m_Right - m_Left;
        auto h = m_Top - m_Bottom;
        m_projMatrix = Matrix4{
            Matrix3{
                2.0f / w,
                2.0f / h,
                2.0f / (m_Near-m_Far),
            },
            Vector3{
                -(m_Right+m_Left) / w,
                -(m_Top+m_Bottom) / h,
                (m_Near+m_Far) / (m_Near-m_Far),
            },
        };
		return m_projMatrix;
	}

	Plane& Camera::getClipPlane(ClipPlane planetype)
	{
		if (m_clipPlaneValid)
			return m_clipPlanes[planetype];
		calcClipPlanes();
		m_clipPlaneValid = true;
		return m_clipPlanes[planetype];
	}

	HW::Matrix4 Camera::ortho(float left, float right, float bottom, float top)
	{
        return Matrix4{
            Matrix3{
                2.0f / (right - left),
                2.0f / (top - bottom),
                -1.0f,
            },
            Vector3{
                -(right + left) / (right - left),
                -(top + bottom) / (top - bottom),
                0.0f,
            },
        };
	}

	//-----------------------------------------------------------------------------------------------
	Matrix4 Camera::getProjectionMatrixDXRH()
	{
		switch (m_ProjType) {
			case PT_PERSPECTIVE: return calcPerspProjMatrixDXRH();
			case PT_ORTHOGONAL:  return calcOrthoProjMatrixOPENGL();
			default: SS_UTIL_UNREACHABLE();
		}
	}

	void Camera::calcClipPlanes()
	{
		if(m_ProjType == PT_PERSPECTIVE)
		{
			/// eye position
			Vector3 eye = getPosition();
			/// direction
			Vector3 mD = getDirection();
			/// up
			Vector3 mU = getUp();
			/// right normalized
			Vector3 mR = mD.crossProduct(mU).hat();
			/// left top near
			Vector3 mLTN = eye +  mD*m_Near + mU*m_Top + mR*m_Left;
			/// left bottom near
			Vector3 mLBN = eye +  mD*m_Near + mU*m_Bottom + mR*m_Left;
			/// right top near
			Vector3 mRTN = eye +  mD*m_Near + mU*m_Top + mR*m_Right;
			/// right bottom near
			Vector3 mRBN = eye +  mD*m_Near + mU*m_Bottom + mR*m_Right;
			/// planes

			m_clipPlanes[CLIP_LEFT] = Plane(mLBN,mLTN,eye);
			m_clipPlanes[CLIP_RIGHT] = Plane(eye,mRTN,mRBN);
			m_clipPlanes[CLIP_TOP] = Plane(eye,mLTN,mRTN);
			m_clipPlanes[CLIP_BOTTOM] = Plane(eye,mRBN,mLBN);
			m_clipPlanes[CLIP_NEAR] = Plane(mD,eye + m_Near*mD);
			m_clipPlanes[CLIP_FAR] = Plane(-mD,eye + m_Far*mD);
		}
		else if(m_ProjType == PT_ORTHOGONAL)
		{
			/// calculate clip planes for orthogonal camera.
			Vector3 eye = getPosition();
			Vector3 mD = getDirection();
			Vector3 mU = getUp();
			Vector3 mR = mD.crossProduct(mU).hat();
			/// left top near
			Vector3 mLTN = eye + mD*m_Near + mU*m_Top + mR*m_Left;
			/// right bottom near
			Vector3 mRBN = eye + mD*m_Near + mU*m_Bottom + mR*m_Right;
			/// planes
			m_clipPlanes[CLIP_LEFT] = Plane(mR,mLTN);
			m_clipPlanes[CLIP_RIGHT] = Plane(-mR,mRBN);
			m_clipPlanes[CLIP_TOP] = Plane(-mU,mLTN);
			m_clipPlanes[CLIP_BOTTOM] = Plane(mU,mRBN);
			m_clipPlanes[CLIP_NEAR] = Plane(mD,eye + m_Near*mD);
			m_clipPlanes[CLIP_FAR] = Plane(-mD,eye + m_Far*mD);
		}
		else
		{
			assert(false && "never reach here.");
		}

	}

	/// test whether bounding box intersects with the frustum.
	unsigned int Camera::intersects(const BoundingBox &box)
	{
		intersect_num ++;
		bool all_side = true;
		for (unsigned int i=0; i<CLIP_PLANE_COUNT; i++)
		{
            Plane::Side side = getClipPlane(ClipPlane(i)).side_of(box);
			if(side == Plane::NEGATIVE_SIDE)
				return false;
            else if(side == Plane::INTERSECTING_PLANE)
				all_side =false;
		}
		if(all_side)
		{
			return 1;
		}else
		{
			return 2;
		}

	}

	/// test whether bounding sphere intersects with the frustum.
	bool Camera::intersects(const Sphere& sphere)
	{
		intersect_num++;
		for (unsigned int i=0; i<CLIP_PLANE_COUNT;i++)
		{
            float dist = getClipPlane(ClipPlane(i)).signed_distance_to(sphere.center);
			if(dist < -sphere.radius)
				return false;
		}
		return true;
	}

	Ray Camera::getCameraToviewportRay(float x,float y)
	{
		Matrix4 inverseVP = (getProjectionMatrixDXRH() *getViewMatrix()).inverse();

		float nx = 2.0f * x - 1.0f;
		float ny = 1.0f - 2.0f * y;
        auto nearPoint = Point3{nx,ny,-1.0f};
        auto midPoint = Point3{nx,ny,0.0f};

        auto rayOrigin = inverseVP * nearPoint;
        auto rayTarget = inverseVP * midPoint;
        auto rayDirection = (rayTarget - rayOrigin).hat();

		Ray ray(rayOrigin,rayDirection);
		return ray;


	}


	void Camera::MoveUp(float d)
	{
		Vector3 pos = getPosition();
		setPosition(pos + d*Vector3(0, 1, 0));

	}

	void Camera::MoveDown(float d)
	{
		Vector3 pos = getPosition();
		setPosition(pos - d*Vector3(0, 1, 0));
	}

	void Camera::MoveLeft(float d)
	{
		Vector3 pos = getPosition();
		Vector3 dir = getDirection();
		Vector3 left = -dir.crossProduct(Vector3(0, 1, 0));
		setPosition(pos + d*left);
	}

	void Camera::MoveRight(float d)
	{
		Vector3 pos = getPosition();
		Vector3 dir = getDirection();
		Vector3 left = -dir.crossProduct(Vector3(0, 1, 0));
		setPosition(pos - d*left);
	}

	void Camera::MoveForward(float d)
	{
		Vector3 pos = getPosition();
		Vector3 dir = getDirection();
		setPosition(pos + d*dir);
	}

	void Camera::MoveBack(float d)
	{
		Vector3 pos = getPosition();
		Vector3 dir = getDirection();
		setPosition(pos - d*dir);
	}

	void Camera::SetPitchYaw(float pitch, float yaw)
	{
		Vector3 front;
		pitch *= constants::pi / 180.0f;
		yaw *= constants::pi / 180.0f;
		front.x = cos(yaw) * cos(pitch);
		front.y = sin(pitch);
		front.z = sin(yaw) * cos(pitch);
		setDirection(front);

	}

	float Camera::getYaw()
	{
		Vector3 dir = getDirection();
		dir.normalize();
		float yaw;
		if (dir.x == 0) {
			yaw = 90;
			if (dir.z < 0)
				yaw += 180;
		}
		else
		{
			yaw = std::atan(dir.z / dir.x) * 180.0f / constants::pi;
			if (dir.x < 0)
				yaw += 180;
			if (yaw < 0)
				yaw += 360;
		}

		return yaw;
	}

	float Camera::getPitch()
	{
		Vector3 dir = getDirection();
		dir.normalize();
		float pitch = std::asin((dir.y)) * 180.0f / constants::pi;
		return pitch;
	}

	void Camera::RotatePitchYaw(float dp, float dy)
	{

		float currentpitch = getPitch();
		float currentyaw = getYaw();
		float pitch = currentpitch+dp;
		float yaw = currentyaw +dy;
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
		SetPitchYaw(pitch, yaw);

	}

	void Camera::establishRenderQueue(SceneNode* node)
	{
		/// we just ignore the null pointer
		/// and do not report an error.
		if(!node) return;
		/// test if this node has entity.
		if(node->hasEntities())
		{
			/// test bounding sphere
			if(intersects(node->getBoundingSphere()))
			{
				/// continue test bounding box
				if(intersects(node->getBoundingBox()))
				{
					const std::list<Entity*>& entities = node->getEntities();
					for (std::list<Entity*>::const_iterator it = entities.begin(); it != entities.end(); it++)
					{
						// test if this entity is visible
						insertMoveableToRenderqueue(*it);

					}
				}
			}
		}

		/// child node process
		if(node->hasChildNodes())
		{
			SceneNode::NodeIterator it = node->getChildBegin();
			SceneNode::NodeIterator end = node->getChildEnd();
			for (; it != end; it++)
			{
				establishRenderQueue(it.get());
			}
		}
	}

	void Camera::insertMoveableToRenderqueue(Movable * moveable)
	{
		if(moveable->visible())
		{
			if(intersects(moveable->getBoundingSphere()))
			{
				if(intersects(moveable->getBoundingBox()))
				{
					Entity * ent = static_cast<Entity*>(moveable);
					assert(ent &&" dynamic cast into entity failed in camera intertmoveablerenderqueue");

					for (std::map<string,Light*>::iterator mLi = m_SceneMgr->m_lightRefList.begin();
						mLi != m_SceneMgr->m_lightRefList.end(); mLi++)
					{
						if(mLi->second->getLightType() == Light::LT_DIRECTIONAL)
						{
							ent->recordLight(mLi->second);
						}
						else if(mLi->second->getLightType() == Light::LT_POINT)
						{
							float dist = (ent->getBoundingSphere().center-
								mLi->second->getPosition()).length();
							if(dist < mLi->second->getMaxDistance())
								ent->recordLight(mLi->second);
						}
						else if(mLi->second->getLightType() == Light::LT_SPOT)
						{
							float dist = (ent->getBoundingSphere().center-
								mLi->second->getPosition()).length();
							if(dist < mLi->second->getMaxDistance())
							{
								/// test the direction
								Vector3 LToE = ent->getBoundingSphere().center -
									mLi->second->getPosition();
								float vcos = LToE.dotProduct(mLi->second->getDirection());
								if(vcos > 0)
								{
									ent->recordLight(mLi->second);
								}
							}
						}
						else
						{
							assert(false && "light type is invalid");
						}
					}

					if(m_RenderQueue.find(ent->getName()) == m_RenderQueue.end())
						m_RenderQueue[ent->getName()] = ent;
					return;
				}
			}
		}
		std::map<string,Entity *>::iterator  it = m_RenderQueue.find(moveable->getName());
		if(m_RenderQueue.end() != it)
			m_RenderQueue.erase(it);

	}

	void Camera::insertVisibleMoveable(Movable * moveable)
	{
		if(moveable->visible())
		{
					Entity * ent = static_cast<Entity*>(moveable);
					assert(ent &&" dynamic cast into entity failed in camera intertmoveablerenderqueue");
					for (std::map<string,Light*>::iterator mLi = m_SceneMgr->m_lightRefList.begin();
						mLi != m_SceneMgr->m_lightRefList.end(); mLi++)
					{
						if(mLi->second->getLightType() == Light::LT_DIRECTIONAL)
						{
							ent->recordLight(mLi->second);
						}
						else if(mLi->second->getLightType() == Light::LT_POINT)
						{
							float dist = (ent->getBoundingSphere().center-
								mLi->second->getPosition()).length();
							if(dist < mLi->second->getMaxDistance())
								ent->recordLight(mLi->second);
						}
						else if(mLi->second->getLightType() == Light::LT_SPOT)
						{
							float dist = (ent->getBoundingSphere().center-
								mLi->second->getPosition()).length();
							if(dist < mLi->second->getMaxDistance())
							{
								/// test the direction
								Vector3 LToE = ent->getBoundingSphere().center -
									mLi->second->getPosition();
								float vcos = LToE.dotProduct(mLi->second->getDirection());
								if(vcos > 0)
								{
									ent->recordLight(mLi->second);
								}
							}
						}
						else
						{
							assert(false && "light type is invalid");
						}
					}

					if(m_RenderQueue.find(ent->getName()) == m_RenderQueue.end())
						m_RenderQueue[ent->getName()] = ent;
					return;
		}


	}

}

