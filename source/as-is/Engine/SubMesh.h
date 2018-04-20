#pragma once
#include "OMesh.h"
#include<string>
#include "NameGenerator.h"
#include <vector>
namespace HW
{
	/** forward declaration
	*/
	class MeshManager;
	/** SubMesh maintain the geometry information.and sub mesh is not sharable.
		so,we do not define a share pointer for this class.
	*/
	class SubMesh : public OMesh
	{
	public:
		/** default constructor.Typically this function should not be used.
		*/
		SubMesh(): OMesh(){}
		/** constructor.function should not be used by user directly.
		*/
		

		virtual ~SubMesh(){
			
		}

		virtual unsigned int isAnimation()
		{
			return 0;
		}

		//virtual void createInternalRes();
		/** return the name of this object
		*/
	//	const String& getName() const { return m_Name; }
		/** name this object
		*/
		void setName(const string& name) { m_Name = name; }

		/**render the object ,need a matrix to calculate the renderable bounding box;
		*/
	//	virtual void render(const Matrix4 & parent_matrix);

		/** get the positions in local coordinate system of the vertex ,
			this may be useful when constructing the bounding box
		*/
		

		SubMesh(const string& name):OMesh(name)
		{
			m_Name = name;
			if(m_Name.empty())
			{
				m_Name = m_nameGenerator->Generate();
			}
		}
		//virtual void setAnimation(bool animation){}
		virtual unsigned int getFrameNum()
		{
			return 0;
		}

	private:
		/** 
		*/
		static NameGenerator* m_nameGenerator;

	};
	
}
