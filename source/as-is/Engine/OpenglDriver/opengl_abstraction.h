#pragma once
#include "OPENGL_Include.h"
#include <vector>
using namespace std;


void CreateVertexBuffer(vector<float>& data,GLuint& vbo);

void CreateVertexBuffer(vector<int>& data, GLuint& vbo);

void CreateIndexBuffer(vector<unsigned int>& data, GLuint& vbo);

void AttachVBOtoVAO_float(GLuint vbo, GLuint vao,int index,int size);

void AttachVBOtoVAO_int(GLuint vbo, GLuint vao, int index, int size);

void CreateVAO(GLuint& vao);

void BindVAO(GLuint vao);

void DrawElements(GLint vao, GLint elementbuffer,int count);

void DrawElementsInstance(GLint vao, GLint elementbuffer, int count, int instanceNum);

void CreateFrameBuffer(GLuint& fbo);

void CreateTexture(GLuint& texid);

void SetProgram2DTexture(int location, GLuint texid, int index);
