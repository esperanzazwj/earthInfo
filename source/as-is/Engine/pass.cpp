#include "pass.h"
#include "engine_struct.h"
#include "VertexDataDesc.h"
#include "EngineUtil.h"
#include "GpuProgram.h"
#include "ResourceManager.h"
#include "RenderStrategy.h"
#include "Texture.h"

void PassManager::ParseAttributeFromJson(rapidjson::Value& attributes, VertexDataDesc* mInputLayout)
{
	mInputLayout->clear();
	for (auto it = attributes.MemberBegin(); it != attributes.MemberEnd(); it++){
		VertexElement* att = new VertexElement;
		string varName = it->name.GetString();
		auto& v = it->value;

		att->name = varName;
		att->type = v["type"].GetString();
		att->semantic = v["semantic"].GetString();
		att->index = v["index"].GetInt();

		mInputLayout->add(att);
	}
}



void PassManager::ParseUniformInputFromJson(rapidjson::Value& uniformmap, Pass* pass)
{
	string varName, semanticName;
	auto uniformgetter = GlobalResourceManager::getInstance().m_PerObjectUniformGetter;
	auto& stringMap = uniformgetter->m_StringSemanticMap;
	for (auto it = uniformmap.MemberBegin(); it != uniformmap.MemberEnd(); it++) {
		varName = it->name.GetString();
		semanticName = it->value.GetString();
		auto itr= stringMap.find(semanticName);
		if (itr != stringMap.end()) {
			PerObjectUniform semantic = itr->second;
			pass->mPerObjectUniform[semantic] = varName;
		}
	}
}

Pass* PassManager::ParsePassFromJsonFile(string filename, const vector<string>& effects)
{
	string file_content = file_util::slurp(filename);
	path_util::Working_Directory_File_Guard wdg{filename};


	rapidjson::Document d;
	d.Parse(file_content.c_str());
	string shader_name = d["name"].GetString();
	Pass* pass = new Pass;
	if (resources.find(shader_name) != resources.end())
	{
		pass->as_pass = resources[shader_name];
		return pass;
	}
	pass->as_pass = new as_Pass;
	auto& grm = GlobalResourceManager::getInstance();

	pass->as_pass->name = shader_name;
	rapidjson::Value shader_desc;
	if (grm.m_platform_info=="GL") 
		shader_desc = d["GLShader"];
	GpuProgram* program = grm.m_GpuProgramFactory->CreateGpuProgram();
	program->effects = effects;
	pass->as_pass->mProgram = program;

	if (shader_desc.HasMember("vertex shader")) {
		string vsPath = (shader_desc["vertex shader"]["path"].GetString());
		program->attachShader(ShaderType::ST_VERTEX, vsPath, vsPath);
	}
	if (shader_desc.HasMember("fragment shader")) {
		string fsPath = (shader_desc["fragment shader"]["path"].GetString());
		program->attachShader(ShaderType::ST_PIXEL, fsPath, fsPath);
	}
	if (shader_desc.HasMember("geometry shader")) {
		string gsPath = (shader_desc["geometry shader"]["path"].GetString());
		program->attachShader(ShaderType::ST_GEOMETRY, gsPath, gsPath);
	}
	if (shader_desc.HasMember("compute shader")) {
		string csPath = (shader_desc["compute shader"]["path"].GetString());
		program->attachShader(ShaderType::ST_COMPUTE, csPath, csPath);
	}
	if (shader_desc.HasMember("merge shader")) {
		string msPath = (shader_desc["merge shader"]["path"].GetString());
		program->attachShader(ShaderType::ST_MERGE, msPath, msPath);
	}
	
	program->CompileAndLink();
	
	//获取attribute

	ParseAttributeFromJson(d["input"]["attribute"], pass->mInputLayout);

	//获取uniform map
	ParseUniformInputFromJson(d["input"]["uniform"], pass);

	//program->SetSemanticVariableMap(pass->mPerObjectUniform);
	return pass;
}

Pass* PassManager::LoadPass(const string& name, const string& path, const vector<string>& effects)
{
	Pass* p = ParsePassFromJsonFile(path, effects);
	p->name = name;
	p->as_pass->name = name;
	add(p->as_pass);
	return p;
}

void Pass::setProgramConstantData(const string & name, Texture * val)
{
	//std::cerr << "Set texture <" << name << "> to [" << val << "]\n";
	if (val == NULL) return;
	//std::cerr << ":: texture name <" << val->name << ">\n";
	UniformData ud;
	ud.type = UT_Texture;
	ud.tex = val;
	mPerFrameUniform[name] = ud;
}

void Pass::setProgramConstantData(const string & name, const void * val, string datatype, unsigned int size)
{
	if (val == NULL) return;
	UniformData ud;
	ud.type = UT_Value;
	ud.datatype = datatype;
	ud.data.resize(size);
	memcpy(&ud.data[0], val, size);
	ud.size = size;
	mPerFrameUniform[name] = ud;
}

void Pass::setSampler(const string& name, Sampler* s)
{
	mSamplerMap[name] = s;
}

Sampler* Pass::getSampler(const string& name)
{
	auto it = mSamplerMap.find(name);
	if (it != mSamplerMap.end())
		return it->second;
	else
		return NULL;
}

void Pass::Render()
{
	RenderStrategy* rs = GlobalResourceManager::getInstance().m_RenderStrategy;
	rs->RenderPass(camera, queue, this, renderTarget);
}

