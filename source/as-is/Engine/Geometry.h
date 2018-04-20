#pragma once
#include "engine_struct.h"
#include "VertexDataDesc.h"
#include "ResourceManager.h"
class GeometryFactory;
class Geometry{
public:
	Geometry(){}
	virtual void Create(as_Geometry* /*geo*/, VertexDataDesc* /*inputlayout*/){}
	virtual void Update(as_Geometry* /*geo*/){}
	virtual bool IsValid(){ return m_bResourceValid; }


protected:
	struct as_Geometry* m_RawGeometryData;
	bool m_bResourceValid;
};

class GeometryFactory{
public:
	virtual Geometry * create() { return NULL; };
	virtual Geometry * create(as_Geometry* /*geo*/, VertexDataDesc* /*inputlayout*/) { return NULL; };
};

class GeometryManager :public ResourceManager<Geometry*>{
public:
	static GeometryManager& getInstance()
	{
		static GeometryManager    instance;
		return instance;
	}

private:

	GeometryManager(){
	}
	GeometryManager(GeometryManager const&);              // Don't Implement.
	void operator=(GeometryManager const&); // Don't implement
};