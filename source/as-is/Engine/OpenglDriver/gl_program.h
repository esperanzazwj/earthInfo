#pragma once
#include "OPENGL_Include.h"
#include "../mathlib.h"
#include <stdexcept>
#include <map>
#include "../GpuProgram.h"
#include "../PreDefine.h"
#include "../ShaderProcessor.h"
#include "../../../util/compat/gl.macro.hpp"
using namespace HW;
using namespace std;


namespace GLSLShader {
	enum GLSLShaderType {
		VERTEX = GL_VERTEX_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER,
		#ifdef SS_UTIL_GL_REQUIRES_ES
			GEOMETRY = 0x8DD9,
			COMPUTE = 0x91B9,
		#else
			GEOMETRY = GL_GEOMETRY_SHADER,
			COMPUTE = GL_COMPUTE_SHADER,
		#endif
	};
};

class GLGpuProgram:public GpuProgram
{
public:
	//继承而来的抽象方法
	virtual bool  CompileAndLink();
	virtual void  UseProgram();
	virtual void releaseInternalRes(){}


	static ShaderProsscer GShaderProsscer;

	int  programID;
	bool linked;

	std::unordered_map<string, int> uniformLocations;
	std::vector<string> effectsNames; // merge use

	GLint  getUniformLocation(const char * name);
	string getExtension(const char * fileName);

	GLGpuProgram();
	~GLGpuProgram();

	void   compileShader(const char *fileName);
	void   compileShader(const char * fileName, GLSLShader::GLSLShaderType type);
	void   compileShader(const string & source, GLSLShader::GLSLShaderType type,
		const char *fileName = NULL);
	void   genMergeShader(const char * fileName, GLSLShader::GLSLShaderType type);

	void   link();
	void   validate();
    void   use();

	int    getHandle();
	bool   isLinked();

	//not used
	void   bindAttribLocation(GLuint location, const char * name);
	//void   bindFragDataLocation(GLuint location, const char * name);

	//important use
	void   setUniform(const char *name, float x, float y, float z);
	void   setUniform(const char *name, const Vector2 & v);
	void   setUniform(const char *name, const Vector3 & v);
	void   setUniform(const char *name, const Vector4 & v);
	void   setUniform(const char *name, const Matrix4 & m);
	void   setUniform(const char *name, const Matrix3 & m);
	void   setUniform(const char *name, float val);
	void   setUniform(const char *name, int val);
	void   setUniform(const char *name, bool val);
	void   setUniform(const char *name, GLuint val);


	//not available
	void   printActiveUniforms();
	void   printActiveUniformBlocks();
	void   printActiveAttribs();

	const char * getTypeString(GLenum type);
};
class GLGpuProgramFactory : public GpuProgramFactory
{
public:
	GLGpuProgramFactory() :GpuProgramFactory(){}
	virtual ~GLGpuProgramFactory(){}
	virtual GpuProgram* CreateGpuProgram(const string& program_name)
	{
		GpuProgram* p = new GLGpuProgram();
		p->SetName(program_name);
		return p;
	}
	virtual GpuProgram* CreateGpuProgram(){ return new GLGpuProgram(); }
};

