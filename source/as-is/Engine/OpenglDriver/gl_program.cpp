#include "gl_program.h"
#include "../../../util/compat/gl.macro.hpp"
#include "../../../wsi/window-system.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../EngineUtil.h"

#include "EGLUtil.h"
#include "../Texture.h"

namespace
{
	char const* glsl_es_header()
	{
		return
			"#version 300 es\n"
			"precision highp float;\n"
			"precision highp usampler2D;\n"
		;
	}

	#ifndef SS_UTIL_GL_REQUIRES_ES
		char const* glsl_desktop_header()
		{
			return
				"#version 430\n"
			;
		}
	#endif

	char const* glsl_version_header()
	{
		#ifdef SS_UTIL_GL_REQUIRES_ES
			return glsl_es_header();
		#else
			auto limited = ss::Window_System::current().context().limited();
			return (limited
				? glsl_es_header()
				: glsl_desktop_header()
			);
		#endif
	}
}

namespace GLSLShaderInfo {
	struct shader_file_extension {
		const char *ext;
		GLSLShader::GLSLShaderType type;
	};

	struct shader_file_extension extensions[] =
	{
		{ ".vs", GLSLShader::VERTEX },
		{ ".vert", GLSLShader::VERTEX },
		{ ".fs", GLSLShader::FRAGMENT },
		{ ".frag", GLSLShader::FRAGMENT },
		{ ".gs", GLSLShader::GEOMETRY },
		{ ".geom", GLSLShader::GEOMETRY },
		//{ ".tcs", GLSLShader::TESS_CONTROL },
		//{ ".tes", GLSLShader::TESS_EVALUATION },
		//{ ".cs", GLSLShader::COMPUTE }
	};
}

GLGpuProgram::GLGpuProgram() : programID(0), linked(false) {

}

GLGpuProgram::~GLGpuProgram() {
	if (programID == 0) return;

	// Query the number of attached shaders
	GLint numShaders = 0;
	glGetProgramiv(programID, GL_ATTACHED_SHADERS, &numShaders);

	// Get the shader names
	GLuint * shaderNames = new GLuint[numShaders];
	glGetAttachedShaders(programID, numShaders, NULL, shaderNames);

	// Delete the shaders
	for (int i = 0; i < numShaders; i++)
		glDeleteShader(shaderNames[i]);

	// Delete the program
	glDeleteProgram(programID);

	delete[] shaderNames;
}

void GLGpuProgram::compileShader(const char * fileName)
{
	int numExts = sizeof(GLSLShaderInfo::extensions) / sizeof(GLSLShaderInfo::shader_file_extension);

	// Check the file name's extension to determine the shader type
	string ext = getExtension(fileName);
	GLSLShader::GLSLShaderType type = GLSLShader::VERTEX;
	bool matchFound = false;
	for (int i = 0; i < numExts; i++) {
		if (ext == GLSLShaderInfo::extensions[i].ext) {
			matchFound = true;
			type = GLSLShaderInfo::extensions[i].type;
			break;
		}
	}

	// If we didn't find a match, throw an exception
	if (!matchFound) {
		string msg = "Unrecognized extension: " + ext;
		assert(msg.c_str());
	}

	// Pass the discovered shader type along
	compileShader(fileName, type);

}

string GLGpuProgram::getExtension(const char * name) {
	string nameStr(name);

	size_t loc = nameStr.find_last_of('.');
	if (loc != string::npos) {
		return nameStr.substr(loc, string::npos);
	}
	return "";
}

void GLGpuProgram::compileShader(
	const char * fileName,
	GLSLShader::GLSLShaderType type)
{
	if (programID <= 0) {
		programID = glCreateProgram();
		if (programID == 0) {
			throw std::runtime_error{"Unable to create shader program."};
		}
	}
	GShaderProsscer.AddShaderFile(fileName);
	string text = GShaderProsscer.GetShaderCode(fileName);
	//string fullpath = HW::path_util::sanitize_runtime_path(fileName);
	//std::ifstream inFile(fullpath, std::ios::in);
	//if (!inFile) {
	//	string message = string("Unable to open: ") + fullpath;
	//	assert(message.c_str());
	//}
	string prefix{glsl_version_header()};
	// Get file contents
	std::stringstream code;
	code << prefix;
	code << text;
	std::cout << "shader code:\n";
	std::cout << code.str() << std::endl;
	compileShader(code.str(), type, fileName);
}

void GLGpuProgram::compileShader(
	const string & source,
	GLSLShader::GLSLShaderType type,
	const char * fileName)
{
	if (programID <= 0) {
		programID = glCreateProgram();
		if (programID == 0) {
			throw std::runtime_error{"Unable to create shader program."};
		}
	}

	GLuint shaderHandle = glCreateShader(type);

	const char * c_code = source.c_str();
	glShaderSource(shaderHandle, 1, &c_code, NULL);

	glCompileShader(shaderHandle);

	int compile_success;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compile_success);

	if (compile_success) {
		glAttachShader(programID, shaderHandle);
	} else {
		int length = 0;
		string logString;
		glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			logString.resize(length);
			int written = 0;
			glGetShaderInfoLog(shaderHandle, length, &written, &logString[0]);
			logString.resize(written);
		}

		string msg;
		if (fileName) {
			msg = string(fileName) + ": shader compliation failed\n";
		} else {
			msg = "Shader compilation failed.\n";
		}
		msg += logString;
		throw std::runtime_error{msg};
	}

}

void GLGpuProgram::genMergeShader(const char * fileName, GLSLShader::GLSLShaderType /*type*/)
{
	cout << "genMergeShader" << endl;
	if (programID <= 0) {
		programID = glCreateProgram();
		if (programID == 0) {
			throw std::runtime_error{"Unable to create shader program."};
		}
	}
	GShaderProsscer.AddShaderFile(fileName);
	string text = GShaderProsscer.GetShaderCode(fileName);

	string prefix{glsl_version_header()};

	// Get file contents
	std::stringstream code, head("\n"), body;

	// insert effects code
	for (auto e : effects)
	{
		if ("gbuffer" == e) continue;

		string configName = e + "/" + e + "_merge.json";
		cout << "configName " << configName << endl;
		string file_content = file_util::slurp(configName);
		rapidjson::Document d;
		d.Parse(file_content.c_str());

		string include = d["include"].GetString();
		head << "#include \"" + e + "/" + include + "\"" << endl;

		string insert = d["insert"].GetString();
		insert = e + "/" + insert;
		string insert_content = file_util::slurp(insert);
		body << insert_content << endl;
	}


	// --- replace include ---
	string temp_inc = "INCLUDE", temp_body = "BODY";
	string mp = R"(<!--([^<]+)-->)";
	map<string, string> replaceMap;
	regex word_regex(mp);

	auto words_begin =
		sregex_iterator(text.begin(), text.end(), word_regex);
	auto words_end = sregex_iterator();
	for (auto i = words_begin; i != words_end; ++i) {
		smatch match = *i;
		string match_str = match.str();
		string tag = match_str.substr(4, match_str.size()).substr(0, match_str.size() - 7);
		if (temp_inc == tag)
			//text = GShaderProsscer.Replace(text, match_str, head.str());
			replaceMap.insert({ match_str, head.str() });
		else if (temp_body == tag)
			//text = GShaderProsscer.Replace(text, match_str, body.str());
			replaceMap.insert({ match_str, body.str() });
	}

	for (auto match : replaceMap)
	{
		text = GShaderProsscer.Replace(text, match.first, match.second);
	}

	code << prefix;
	code << text;

	string processedStr = GShaderProsscer.ParseFileText(fileName, code.str());

	{
		std::ofstream ofs{"debug_merge.fs"};
		ofs << processedStr << "\n";
	}

	compileShader(processedStr, GLSLShader::GLSLShaderType::FRAGMENT, fileName);
	cout << "genMergeShader1" << endl;
}

void GLGpuProgram::link()
{
	if (linked)
		throw std::runtime_error{"Linked program cannot be linked again."};;
	if (programID <= 0)
		throw std::runtime_error{"Program has not been compiled."};

	glLinkProgram(programID);

	int link_success = 0;
	glGetProgramiv(programID, GL_LINK_STATUS, &link_success);
	if (link_success) {
		uniformLocations.clear();
		linked = true;
	} else {
		int length = 0;
		string logString;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			logString.resize(length);
			int written = 0;
			glGetProgramInfoLog(programID, length, &written, &logString[0]);
			logString.resize(written);
		}

		auto msg = "Program link failed:\n" + logString;
		throw std::runtime_error{msg};
	}
}

void PrintActiveUniforms(int programID) {
	int total = -1;
	glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &total);
	for (int i = 0; i < total; ++i) {
		int name_len = -1, num = -1;
		GLenum type = GL_ZERO;
		char name[100];
		glGetActiveUniform(programID, GLuint(i), sizeof(name) - 1,
			&name_len, &num, &type, name);
		name[name_len] = 0;
		GLuint location = glGetUniformLocation(programID, name);
		cout << name << " " << location << endl;
	}
}
void GLGpuProgram::use()
{
	if (programID <= 0 || (!linked))
		throw std::runtime_error{"Shader has not been linked"};
	glUseProgram(programID);

	//PrintActiveUniforms(programID);

}

int GLGpuProgram::getHandle()
{
	return programID;
}

bool GLGpuProgram::isLinked()
{
	return linked;
}

void GLGpuProgram::bindAttribLocation(GLuint location, const char * name)
{
	glBindAttribLocation(programID, location, name);
}



void GLGpuProgram::setUniform(const char *name, float x, float y, float z)
{
	GLint loc = getUniformLocation(name);
	glUniform3f(loc, x, y, z);
}

void GLGpuProgram::setUniform(const char *name, const Vector3 & v)
{
	this->setUniform(name, v.x, v.y, v.z);
}

void GLGpuProgram::setUniform(const char *name, const Vector4 & v)
{
	GLint loc = getUniformLocation(name);
	glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void GLGpuProgram::setUniform(const char *name, const Vector2 & v)
{
	GLint loc = getUniformLocation(name);
	glUniform2f(loc, v.x, v.y);
}

void GLGpuProgram::setUniform(const char *name, const Matrix4 & m)
{
	GLint loc = getUniformLocation(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, m.ptr());
}

void GLGpuProgram::setUniform(const char *name, const Matrix3 & m)
{
	GLint loc = getUniformLocation(name);
	glUniformMatrix3fv(loc, 1, GL_FALSE, m.ptr());
}

void GLGpuProgram::setUniform(const char *name, float val)
{
	GLint loc = getUniformLocation(name);
	glUniform1f(loc, val);
}

void GLGpuProgram::setUniform(const char *name, int val)
{
	GLint loc = getUniformLocation(name);
	glUniform1i(loc, val);
}

void GLGpuProgram::setUniform(const char *name, GLuint val)
{
	GLint loc = getUniformLocation(name);
	glUniform1ui(loc, val);
}

void GLGpuProgram::setUniform(const char *name, bool val)
{
	int loc = getUniformLocation(name);
	glUniform1i(loc, val);
}

void GLGpuProgram::printActiveUniforms() {
	//GLint numUniforms = 0;
	//glGetProgramInterfaceiv(handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

	//GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };

	//printf("Active uniforms:\n");
	//for (int i = 0; i < numUniforms; ++i) {
	//	GLint results[4];
	//	glGetProgramResourceiv(handle, GL_UNIFORM, i, 4, properties, 4, NULL, results);

	//	if (results[3] != -1) continue;  // Skip uniforms in blocks
	//	GLint nameBufSize = results[0] + 1;
	//	char * name = new char[nameBufSize];
	//	glGetProgramResourceName(handle, GL_UNIFORM, i, nameBufSize, NULL, name);
	//	printf("%-5d %s (%s)\n", results[2], name, getTypeString(results[1]));
	//	delete[] name;
	//}
}

void GLGpuProgram::printActiveUniformBlocks() {
	//GLint numBlocks = 0;

	//glGetProgramInterfaceiv(handle, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);
	//GLenum blockProps[] = { GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH };
	//GLenum blockIndex[] = { GL_ACTIVE_VARIABLES };
	//GLenum props[] = { GL_NAME_LENGTH, GL_TYPE, GL_BLOCK_INDEX };

	//for (int block = 0; block < numBlocks; ++block) {
	//	GLint blockInfo[2];
	//	glGetProgramResourceiv(handle, GL_UNIFORM_BLOCK, block, 2, blockProps, 2, NULL, blockInfo);
	//	GLint numUnis = blockInfo[0];

	//	char * blockName = new char[blockInfo[1] + 1];
	//	glGetProgramResourceName(handle, GL_UNIFORM_BLOCK, block, blockInfo[1] + 1, NULL, blockName);
	//	printf("Uniform block \"%s\":\n", blockName);
	//	delete[] blockName;

	//	GLint * unifIndexes = new GLint[numUnis];
	//	glGetProgramResourceiv(handle, GL_UNIFORM_BLOCK, block, 1, blockIndex, numUnis, NULL, unifIndexes);

	//	for (int unif = 0; unif < numUnis; ++unif) {
	//		GLint uniIndex = unifIndexes[unif];
	//		GLint results[3];
	//		glGetProgramResourceiv(handle, GL_UNIFORM, uniIndex, 3, props, 3, NULL, results);

	//		GLint nameBufSize = results[0] + 1;
	//		char * name = new char[nameBufSize];
	//		glGetProgramResourceName(handle, GL_UNIFORM, uniIndex, nameBufSize, NULL, name);
	//		printf("    %s (%s)\n", name, getTypeString(results[1]));
	//		delete[] name;
	//	}

	//	delete[] unifIndexes;
	//}
}

void GLGpuProgram::printActiveAttribs() {
	//GLint numAttribs;
	//glGetProgramInterfaceiv(handle, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttribs);

	//GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION };

	//printf("Active attributes:\n");
	//for (int i = 0; i < numAttribs; ++i) {
	//	GLint results[3];
	//	glGetProgramResourceiv(handle, GL_PROGRAM_INPUT, i, 3, properties, 3, NULL, results);

	//	GLint nameBufSize = results[0] + 1;
	//	char * name = new char[nameBufSize];
	//	glGetProgramResourceName(handle, GL_PROGRAM_INPUT, i, nameBufSize, NULL, name);
	//	printf("%-5d %s (%s)\n", results[2], name, getTypeString(results[1]));
	//	delete[] name;
	//}
}

const char * GLGpuProgram::getTypeString(GLenum type) {
	// There are many more types than are covered here, but
	// these are the most common in these examples.
	switch (type) {
	case GL_FLOAT:
		return "float";
	case GL_FLOAT_VEC2:
		return "vec2";
	case GL_FLOAT_VEC3:
		return "vec3";
	case GL_FLOAT_VEC4:
		return "vec4";
	//case GL_DOUBLE:
	//	return "double";
	case GL_INT:
		return "int";
	case GL_UNSIGNED_INT:
		return "unsigned int";
	case GL_BOOL:
		return "bool";
	case GL_FLOAT_MAT2:
		return "mat2";
	case GL_FLOAT_MAT3:
		return "mat3";
	case GL_FLOAT_MAT4:
		return "mat4";
	default:
		return "?";
	}
}

void GLGpuProgram::validate()
{
	if (!isLinked())
		assert("glprogram_error");

	GLint status;
	glValidateProgram(programID);
	glGetProgramiv(programID, GL_VALIDATE_STATUS, &status);

	if (GL_FALSE == status) {
		// Store log and return false
		int length = 0;
		string logString;

		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length);

		if (length > 0) {
			char * c_log = new char[length];
			int written = 0;
			glGetProgramInfoLog(programID, length, &written, c_log);
			logString = c_log;
			delete[] c_log;
		}

		throw std::runtime_error{"Program failed to validate: " + logString};
	}
}

int GLGpuProgram::getUniformLocation(const char * name)
{
	std::unordered_map<string, int>::iterator pos;
	pos = uniformLocations.find(name);

	if (pos == uniformLocations.end()) {
		uniformLocations[name] = glGetUniformLocation(programID, name);
	}

	return uniformLocations[name];
}

void GLGpuProgram::UseProgram()
{
	use();
	GL_CHECK();
}

bool GLGpuProgram::CompileAndLink()
{
	GL_CHECK();
	if (m_ShaderPath[ShaderType::ST_VERTEX] != "")
		compileShader(m_ShaderPath[ShaderType::ST_VERTEX].c_str(),GLSLShader::VERTEX);
	if (m_ShaderPath[ShaderType::ST_PIXEL] != "")
		compileShader(m_ShaderPath[ShaderType::ST_PIXEL].c_str(), GLSLShader::FRAGMENT);
	if(m_ShaderPath[ShaderType::ST_GEOMETRY]!="")
		compileShader(m_ShaderPath[ShaderType::ST_GEOMETRY].c_str(), GLSLShader::GEOMETRY);
	if (m_ShaderPath[ShaderType::ST_COMPUTE] != "")
		compileShader(m_ShaderPath[ShaderType::ST_COMPUTE].c_str(), GLSLShader::COMPUTE);
	if (m_ShaderPath[ShaderType::ST_MERGE] != "")
		genMergeShader(m_ShaderPath[ShaderType::ST_MERGE].c_str(), GLSLShader::FRAGMENT);

	link();
	validate();
	GL_CHECK();
	return true;
}


ShaderProsscer GLGpuProgram::GShaderProsscer;

