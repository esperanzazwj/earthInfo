#include "opengl_abstraction.h"

#include "EGLUtil.h"



void CreateVertexBuffer(vector<float>& data, GLuint& vbo)
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof (float), &data[0], GL_STATIC_DRAW);
	GL_CHECK();
}

void CreateVertexBuffer(vector<int>& data, GLuint& vbo)
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof (int), &data[0], GL_STATIC_DRAW);
	GL_CHECK();
}

void CreateIndexBuffer(vector<unsigned int>& data, GLuint& vbo)
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), &data[0], GL_STATIC_DRAW);
	GL_CHECK();
}

void AttachVBOtoVAO_float(GLuint vbo, GLuint vao, int index, int size)
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
	glEnableVertexAttribArray(index);
	GL_CHECK();
}

void AttachVBOtoVAO_int(GLuint vbo, GLuint vao, int index, int size)
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribIPointer(index, size, GL_INT, 0, (GLubyte*)NULL);
	glEnableVertexAttribArray(index);
	GL_CHECK();
}

void CreateVAO(GLuint& vao)
{
	glGenVertexArrays(1, &vao);
	GL_CHECK();
}

void BindVAO(GLuint vao)
{
	glBindVertexArray(vao);
	GL_CHECK();
}

void DrawElements(GLint vao, GLint elementbuffer, int count)
{
	GL_CHECK(glBindVertexArray(vao));
	GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer));
	GL_CHECK(glDrawElements(
		GL_TRIANGLES,      // mode
		count,			   // count
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
		));
}

void DrawElementsInstance(GLint vao, GLint elementbuffer, int count, int instanceNum)
{
	GL_CHECK(glBindVertexArray(vao));
	GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer));
	GL_CHECK(glDrawElementsInstanced(
		GL_TRIANGLES,      // mode
		count,			   // count
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
		, instanceNum));
}

void CreateFrameBuffer(GLuint& fbo)
{
	GL_CHECK(glGenFramebuffers(1, &fbo));
}

void CreateTexture(GLuint& texid)
{
	GL_CHECK(glGenTextures(1, &texid));
}

void SetProgram2DTexture(int location, GLuint texid, int index)
{
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, texid);
	glUniform1i(location, index);
	GL_CHECK();
}
