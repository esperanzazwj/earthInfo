#include "PerObjectUniform.h"
#include "Camera.h"
#include "Texture.h"
#include "TextureManager.h"
#include "EngineUtil.h"
#include "UniformSetter.h"
#include "RenderSystem.h"

void PerObjectUniformGetter::GetAndSetUniforms(unordered_map<PerObjectUniform, string>& perObjUniforms, Pass* pass, Entity* entity,
	Camera* camera, as_Material* material,GpuProgram* program,int texture_index)
{
	auto uniformsetter = GlobalResourceManager::getInstance().m_UniformSetter;
	auto rhi = GlobalResourceManager::getInstance().m_RenderSystem;
	for (auto& x : perObjUniforms) {
		PerObjectUniform sem = x.first;
		string name = x.second;
		Matrix4 mat4;
		Vector4 v;
		Vector3 pos;
		switch (sem)
		{
		case GVS_WORLD_MATRIX: {
			mat4 = GetWorldMatrix(entity);
			rhi->SetUniform(name, program, "mat4", 1, &mat4[0][0]);
		}break;
		case GVS_VIEW_MATRIX: {
			mat4 = GetViewMatrix(camera);
			rhi->SetUniform(name, program, "mat4", 1, &mat4[0][0]);
		}break;
		case GVS_PROJECTION_MATRIX: {
			mat4 = GetProjectionMatrix(camera);
			rhi->SetUniform(name, program, "mat4", 1, &mat4[0][0]);
		}break;
		case GVS_CAMERA_POSITION: {
			pos = GetCameraPosition(camera);
			rhi->SetUniform(name, program, "vec3", 1, &pos[0]);
		}break;
		case GVS_MATERIAL_AMBIENT:
		case GVS_MATERIAL_DIFFUSE:
		case GVS_MATERIAL_SPECULAR:
		case GVS_MATERIAL_EMISSIVE:
		case GVS_MATERIAL_TRANSPARENT:
		case GVS_MATERIAL_OPACITY:
		case GVS_MATERIAL_SHININESS:
		case GVS_MATERIAL_ROUGHNESS:
		case GVS_MATERIAL_METALNESS: {
			v = GetMaterialValue(material, sem);
			rhi->SetUniform(name, program, "vec4", 1, &v[0]);
		}break;
		case GVS_TEXTURE_OPACITY:
		case GVS_TEXTURE_DIFFUSE:
		case GVS_TEXTURE_AMBIENT:
		case GVS_TEXTURE_SPECULAR:
		case GVS_TEXTURE_EMISSIVE:
		case GVS_TEXTURE_HEIGHT:
		case GVS_TEXTURE_NORMAL:
		case GVS_TEXTRUE_SHININESS:
		case GVS_TEXTURE_DISPLACEMENT:
		case GVS_TEXTURE_LIGHTMAP:
		case GVS_TEXTURE_REFLECTION:
		case GVS_TEXTURE_ROUGHNESS:
		case GVS_TEXTURE_METALNESS:
		case GVS_TEXTURE_UNKNOWN:
		case GVS_TEXTURE_LEAN1:
		case GVS_TEXTURE_LEAN2:
		{
			Texture* tex = GetMaterialTexture(material, sem);
			uniformsetter->SetProgramTexture(name, program, tex, texture_index);
			uniformsetter->SetTexureSampler(name, pass, tex, texture_index);
			texture_index++;
		}
			break;
		case GVS_EFFECT_MASK: {
			rhi->SetUniform(name, program, "uint", 1, entity->GetEffectMask());
		}
			break;
		default:
			break;
		}

	}
}

Matrix4 PerObjectUniformGetter::GetWorldMatrix(Entity* entity)
{
	if (entity == NULL)
		return {};
	else
		return entity->getParent()->getWorldMatrix();
}

Matrix4 PerObjectUniformGetter::GetViewMatrix(Camera* camera)
{
	if (!camera)
		return {};
	return camera->getViewMatrix();
}

Matrix4 PerObjectUniformGetter::GetProjectionMatrix(Camera* camera)
{
	if (!camera)
		return {};
	return camera->getProjectionMatrixDXRH();
}

HW::Vector3 PerObjectUniformGetter::GetCameraPosition(Camera* camera)
{
	if (!camera)
		return Vector3(0,0,0);
	return camera->getPosition();
}

Vector4 PerObjectUniformGetter::GetMaterialValue(as_Material* mat, PerObjectUniform sem)
{
	Vector4 v(1.0);
	if (mat == NULL) return v;
	switch (sem)
	{
	case GVS_MATERIAL_AMBIENT:
		v = Vector4{mat->ambient, 1.0f};
		break;
	case GVS_MATERIAL_DIFFUSE:
	{
		v = Vector4{mat->diffuse, 1.0f};

		//temporally use to indicate if has diffuse texture
		auto it = mat->textures.find("diffuse");
		if (it == mat->textures.end())
			v.w = 0;
		else
			v.w = 1;
	}
		break;
	case GVS_MATERIAL_SPECULAR:
		v = Vector4{mat->specular, 1.0f};
		break;
	case GVS_MATERIAL_EMISSIVE:
		v = Vector4{mat->emissive, 1.0f};
		break;
	case GVS_MATERIAL_TRANSPARENT:
		v = Vector4{mat->transparent, 1.0f};
		break;
	case GVS_MATERIAL_OPACITY:
		v = Vector4(mat->opacity);
		break;
	case GVS_MATERIAL_SHININESS:
		v = Vector4(mat->shininess);
		break;
	case GVS_MATERIAL_ROUGHNESS:
		v = Vector4(mat->roughness);
		break;
	case GVS_MATERIAL_METALNESS:
		v = Vector4(mat->metalness);
		break;
	default:
		break;
	}
	return v;
}

Texture* PerObjectUniformGetter::GetMaterialTexture(as_Material* mat, PerObjectUniform sem)
{
	auto& tm = TextureManager::getInstance();
	Texture* whiteTex = tm.get("white_texture.png");
	if (mat == NULL) return whiteTex;
	string textureType;
	switch (sem)
	{
	case GVS_TEXTURE_OPACITY:
		textureType = "opacity";
		break;
	case GVS_TEXTURE_DIFFUSE:
		textureType = "diffuse";
		break;
	case GVS_TEXTURE_AMBIENT:
		textureType = "ambient";
		break;
	case GVS_TEXTURE_SPECULAR:
		textureType = "specular";
		break;
	case GVS_TEXTURE_EMISSIVE:
		textureType = "emissive";
		break;
	case GVS_TEXTURE_HEIGHT:
		textureType = "height";
		break;
	case GVS_TEXTURE_NORMAL:
		textureType = "normal";
		break;
	case GVS_TEXTRUE_SHININESS:
		textureType = "shininess";
		break;
	case GVS_TEXTURE_DISPLACEMENT:
		textureType = "displacement";
		break;
	case GVS_TEXTURE_LIGHTMAP:
		textureType = "lightmap";
		break;
	case GVS_TEXTURE_REFLECTION:
		textureType = "reflection";
		break;
	case GVS_TEXTURE_ROUGHNESS:
		textureType = "roughness";
		break;
	case GVS_TEXTURE_METALNESS:
		textureType = "metalness";
		break;
	case GVS_TEXTURE_UNKNOWN:
		break;
	case GVS_TEXTURE_LEAN1:
		textureType = "lean1";
		break;
	case GVS_TEXTURE_LEAN2:
		textureType = "lean2";
		break;
	default:
		break;
	}
	auto texIterator = mat->textures.find(textureType);
	if (texIterator == mat->textures.end())
		return whiteTex;
    Texture* tex = tm.get(texIterator->second.path);
	return tex;
}

void PerObjectUniformGetter::InitStringEnumMap()
{
	m_StringSemanticMap.clear();
	m_StringSemanticMap["GVS_WORLD_MATRIX"] = GVS_WORLD_MATRIX;
	m_StringSemanticMap["GVS_VIEW_MATRIX"] = GVS_VIEW_MATRIX;
	m_StringSemanticMap["GVS_PROJECTION_MATRIX"] = GVS_PROJECTION_MATRIX;
	m_StringSemanticMap["GVS_CAMERA_POSITION"] = GVS_CAMERA_POSITION;
	m_StringSemanticMap["GVS_MATERIAL_AMBIENT"] = GVS_MATERIAL_AMBIENT;
	m_StringSemanticMap["GVS_MATERIAL_DIFFUSE"] = GVS_MATERIAL_DIFFUSE;
	m_StringSemanticMap["GVS_MATERIAL_SPECULAR"] = GVS_MATERIAL_SPECULAR;
	m_StringSemanticMap["GVS_MATERIAL_EMISSIVE"] = GVS_MATERIAL_EMISSIVE;
	m_StringSemanticMap["GVS_MATERIAL_TRANSPARENT"] = GVS_MATERIAL_TRANSPARENT;
	m_StringSemanticMap["GVS_MATERIAL_OPACITY"] = GVS_MATERIAL_OPACITY;
	m_StringSemanticMap["GVS_MATERIAL_SHININESS"] = GVS_MATERIAL_SHININESS;
	m_StringSemanticMap["GVS_MATERIAL_ROUGHNESS"] = GVS_MATERIAL_ROUGHNESS;
	m_StringSemanticMap["GVS_MATERIAL_METALNESS"] = GVS_MATERIAL_METALNESS;
	m_StringSemanticMap["GVS_TEXTURE_DIFFUSE"] = GVS_TEXTURE_DIFFUSE;
	m_StringSemanticMap["GVS_TEXTURE_HEIGHT"] = GVS_TEXTURE_HEIGHT;
	m_StringSemanticMap["GVS_TEXTURE_AMBIENT"] = GVS_TEXTURE_AMBIENT;
	m_StringSemanticMap["GVS_TEXTURE_SPECULAR"] = GVS_TEXTURE_SPECULAR;
	m_StringSemanticMap["GVS_TEXTURE_NORMAL"] = GVS_TEXTURE_NORMAL;
	m_StringSemanticMap["GVS_TEXTURE_HEIGHT"] = GVS_TEXTURE_HEIGHT;
	m_StringSemanticMap["GVS_TEXTURE_ROUGHNESS"] = GVS_TEXTURE_ROUGHNESS;
	m_StringSemanticMap["GVS_TEXTURE_OPACITY"] = GVS_TEXTURE_OPACITY;
	m_StringSemanticMap["GVS_TEXTURE_EMISSIVE"] = GVS_TEXTURE_EMISSIVE;
	m_StringSemanticMap["GVS_TEXTURE_DISPLACEMENT"] = GVS_TEXTURE_DISPLACEMENT;
	m_StringSemanticMap["GVS_TEXTRUE_SHININESS"] = GVS_TEXTRUE_SHININESS;
	m_StringSemanticMap["GVS_TEXTURE_LIGHTMAP"] = GVS_TEXTURE_LIGHTMAP;
	m_StringSemanticMap["GVS_TEXTURE_REFLECTION"] = GVS_TEXTURE_REFLECTION;
	m_StringSemanticMap["GVS_TEXTURE_UNKNOWN"] = GVS_TEXTURE_UNKNOWN;
	m_StringSemanticMap["GVS_TEXTURE_LEAN1"] = GVS_TEXTURE_LEAN1;
	m_StringSemanticMap["GVS_TEXTURE_LEAN2"] = GVS_TEXTURE_LEAN2;
	m_StringSemanticMap["GVS_SHADOW_INDEX"] = GVS_SHADOW_INDEX;
	m_StringSemanticMap["GVS_EFFECT_MASK"] = GVS_EFFECT_MASK;
}

