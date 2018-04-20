#pragma once
#include<string>
#include "mathlib.h"
#include "Movable.h"
#include "SceneManager.h"
#include <vector>
#include <map>
#include "PreDefine.h"
namespace HW
{

	struct  ViewPort
	{
		bool used;
		int x;
		int y;
		int width;
		int height;

		ViewPort()
		{
			used = false;
			x = y = width = height = 0;
		}

	};

	class Camera : public Movable
	{
	public:
		/// projection approach enumeration.
		enum ProjectionType
		{
			PT_PERSPECTIVE = 1,
			PT_ORTHOGONAL = 2
		};
		/// clip plane enum
		enum ClipPlane
		{
			CLIP_TOP,
			CLIP_BOTTOM,
			CLIP_LEFT,
			CLIP_RIGHT,
			CLIP_NEAR,
			CLIP_FAR,
			CLIP_PLANE_COUNT
		};
		//============================ common setting ==========================
		/// for fast initialization
		Camera():target_to_use(NULL),renderable_test(false){
		intersect_num = 0;
		}
		Camera(const string& name,SceneManager* creator = NULL);
		virtual ~Camera();
		/// this type can't be changed once set.
		void setProjectionType(ProjectionType type);
		// local space,default (0,0,0)
		void setPosition(const Vector3 &pos);
		// return position in global space.
		Vector3 getPosition();
		/** set camera direction in local space,the default direction is (0,0,-1)
		*/
		void setDirection(const Vector3 &dir);
		/** get camera direction in global space. 
		*/
		Vector3 getDirection();
		/// set position and camera coordinate system at one time
		void lookAt(const Vector3 &eye,const Vector3 &focus,const Vector3 &up);
		/// look at bounding box
		void lookAtBB(const Vector3 &eye, const BoundingBox& bb, const Vector3 &up);
		// local space,default (0,0,1),this function must be called after 'setDirection' is called.
		void setUp(const Vector3 &up);
		// return up in global space.
		Vector3 getUp();
		
		/** set the frustum of this camera
		*/
		void setFrustum(float left,float right,float bottom,float top,float tnear,float tfar);

		/// set perspective.angle in degree unit.
		void setPerspective(float fovy,float spec,float tnear,float tfar);
		void setProjectionMatrix(Matrix4 proj);
		void setViewMatrix(Matrix4 view);

		float getFov();
		float getNear();
		float getFar();
		float getLeft();
		float getRight();
		float getBottom();
		float getTop();
		float getAspect();

		Plane& getClipPlane(ClipPlane planetype);
		Matrix4 ortho(float left,float right,
			float bottom,
			float top
			);
		/// return a projection matrix specially for D3D in right handed system.
		Matrix4 getProjectionMatrixDXRH();
		/// consider with depth range.
		Matrix4 getProjectionMatrixDepth();
		/// return view matrix
		/** in our render frame ,we always assume that vertex data are established
		    in a right handed coordinate system,follow the convention of OpenGL.
		*/
		Matrix4 getViewMatrix();

		Ray getCameraToviewportRay(float x,float y);

		//move methods
		void MoveUp(float d);
		void MoveDown(float d);
		void MoveLeft(float d);
		void MoveRight(float d);
		void MoveForward(float d);
		void MoveBack(float d);
		void SetPitchYaw(float pitch, float yaw);
		float getYaw();
		float getPitch();
		void RotatePitchYaw(float dp, float dy);

		Vector3 m_Position;
	protected:
		/// members for frustum 
		float m_Left,m_Right,m_Top,m_Bottom,m_Near,m_Far,m_fov, m_spec;
		
		Vector3 m_DerivedPosition;
		// hint whether current global position is valid.
		bool    m_derivedPosValid;
		// camera coordinate system
		// local 
		Vector3 m_Direction;
		Vector3 m_Up;
		// global
		Vector3 m_DerivedDirection;
		Vector3 m_DerivedUp;
		// hint whether local coordinate system is valid.
		bool m_localCoordValid;
		/**  matrix for rendering use.
		*/
		Matrix4 m_viewMatrix;
		Matrix4 m_projMatrix;
		// hint ,default false.
		bool m_viewMatrixValid;
		bool m_projMatrixValid;
		/// clip planes 
		/**  six plane for clipping.
			index                  plane
			0						top
			1						bottom
			2						left
			3						right
			4						near
			5						far
		*/
		Plane  m_clipPlanes[CLIP_PLANE_COUNT];
		bool   m_clipPlaneValid;


		//============================== just for rendering =============================
	public:
		// a new render_queue with vector;
		std::vector<Entity *> v_render_queue;
		unsigned int intersect_num;

		/// hint whether render queue has been established.
		bool renderQueueEstablished() const { return m_renderQueueValid; }

		void establishRenderQueue(bool /*v*/)
		{
			m_renderQueueValid = true;
		}

		/// test whether frustum intersects with a bounding box.internal use.
		/// use unsigned int tor present full partial and none;
		unsigned int intersects(const BoundingBox &box);
		/// test whether frustum intersect with a bounding sphere.internal use.
		bool intersects(const Sphere &sphere);
		// insert the moveable to the renderqueue
		void insertMoveableToRenderqueue(Movable * moveable);
		// insert the moveable already being test visibility to the renderqueue
		void insertVisibleMoveable(Movable * moveable);
		/// tell the camera which view port it belongs to.
		void setViewport(bool use_view , int x = 0,int y = 0,int width = 0,int height = 0) {  
			m_viewport.used = use_view;
			m_viewport.x = x;
			m_viewport.y = y;
			m_viewport.width = width;
			m_viewport.height = height;
		}

		const  ViewPort & getViewPort() const
		{
			return m_viewport;
		}

	protected:
		/// build the render queue.
		/** this function traverse the scene graph to collect the render items
			by testing a series of conditions(e.g. bounding box intersections).
		*/
		void establishRenderQueue(SceneNode* node);
		/// clear render queue,when one frame is rendered over,render queue will be cleared.
		void clearRenderQueue()
		{
			m_RenderQueue.clear();
			m_renderQueueValid = false;
		}
		virtual void moveActionImp()
		{
			m_derivedPosValid = false;
			m_localCoordValid = false;
			m_viewMatrixValid = false;
			m_clipPlaneValid = false;
		}
		/// for camera,bounding box makes no sense.
		virtual void getBoundingBoxImp() {}
		/// for camera, bounding sphere makes no sense.
		virtual void getBoundingSphereImp() {}
		/// calculate local coordinate system. internal use.
		void calcLocalSystem();
		/// calculate clip plane .internal use.
		void calcClipPlanes();
		
		/// compute the perspective projection matrix in a D3D context.internal use.
		Matrix4 calcPerspProjMatrixDXRH();
		/// compute the orthogonal projection matrix in a D3D context.internal use.
		Matrix4 calcOrthoProjMatrixOPENGL();

		
		

	
	protected:

		std::map<string,Entity*> m_RenderQueue;
		
		/// hint whether render queue is valid for rendering.
		bool m_renderQueueValid;

		ProjectionType m_ProjType;
		RenderTarget * target_to_use ;

		bool renderable_test;

		ViewPort m_viewport;

		
		
	};

}
