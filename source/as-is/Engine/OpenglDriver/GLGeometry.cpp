#include "GLGeometry.h"
#include "opengl_abstraction.h"

void GLGeometry::Create(as_Geometry* geo, VertexDataDesc* inputlayout)
{
	m_RawGeometryData = geo;
	CreateVAO(gldata.vao);
	BindVAO(gldata.vao);
	auto AttributeList = inputlayout->getElements();
	for (auto& att : AttributeList){
		int size;
		string type;
		inputlayout->getAttributeDataType(att, type, size);
		vector<float>* data = NULL;
		if (att->semantic == "POSITION"){ data = &geo->position; }
		if (att->semantic == "NORMAL"){ data = &geo->normal; }
		if (att->semantic == "TEXCOORD"){ data = &geo->texcoord; }
		if (att->semantic == "BONEWEIGHT"){ data = &geo->boneWeight; }
		if (att->semantic == "TANGENT") { data = &geo->tangent; }
		if (att->semantic == "BITANGENT") { data = &geo->bitangent; }
		if (data != NULL&&data->size() > 0){
			CreateVertexBuffer(*data, gldata.vbo[att->index]);
			AttachVBOtoVAO_float(gldata.vbo[att->index], gldata.vao, att->index, size);
		}

		if (att->semantic == "BONEID"){
			if (geo->has_skeleton&&geo->boneID.size() > 0){
				CreateVertexBuffer(geo->boneID, gldata.vbo[att->index]);
				AttachVBOtoVAO_int(gldata.vbo[att->index], gldata.vao, att->index, size);
			}
		}
	}
	if (geo->indices.size() > 0){
		CreateIndexBuffer(geo->indices, gldata.elementbuffer);
		gldata.indices_size = (int)geo->indices.size();
	}
	m_bResourceValid = true;
}

void GLGeometry::AttachInstanceBufferFloat(int /*size*/, int attribLocation, int attribNum, void* /*data*/)
{
	assert(m_bResourceValid == true);
	BindVAO(gldata.vao);
	for (int i = 0; i < attribNum; i++) {
		int location = i + attribLocation;
		glEnableVertexAttribArray(location);
		glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix4),(GLvoid*)(sizeof(GLfloat) * i * 4));
		glVertexAttribDivisor(location + i, 1);
	}
}


void GLGeometry::Draw()
{
	if (m_bResourceValid){
		DrawElements(gldata.vao, gldata.elementbuffer, gldata.indices_size);
	}
}
void GLGeometry::DrawInstance(int num)
{
	if (m_bResourceValid) {
		DrawElementsInstance(gldata.vao, gldata.elementbuffer, gldata.indices_size,num);
	}
}
Geometry* GLGeometryFactory::create() {
	return new GLGeometry;
}

Geometry* GLGeometryFactory::create(as_Geometry* geo, VertexDataDesc* inputlayout) {
	Geometry* g = new GLGeometry;
	g->Create(geo, inputlayout);
	return g;
}
