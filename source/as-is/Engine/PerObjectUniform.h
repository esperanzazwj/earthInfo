#pragma once

#include "PreDefine.h"
#include "mathlib.h"
#include <string>
#include <map>
#include <unordered_map>
using namespace HW;

// a pool to temp store uniform
enum PerObjectUniform
{
	GVS_WORLD_MATRIX,
	GVS_VIEW_MATRIX,
	GVS_PROJECTION_MATRIX,// this two matrix not per obj yet
	GVS_CAMERA_POSITION,
	// material
	GVS_MATERIAL_AMBIENT,
	GVS_MATERIAL_DIFFUSE,
	GVS_MATERIAL_SPECULAR,
	GVS_MATERIAL_EMISSIVE,
	GVS_MATERIAL_TRANSPARENT,
	GVS_MATERIAL_OPACITY,
	GVS_MATERIAL_SHININESS,
	GVS_MATERIAL_ROUGHNESS,
	GVS_MATERIAL_METALNESS,

	// texture map
	GVS_TEXTURE_OPACITY,
	GVS_TEXTURE_DIFFUSE,
	GVS_TEXTURE_AMBIENT,
	GVS_TEXTURE_SPECULAR,
	GVS_TEXTURE_EMISSIVE,
	GVS_TEXTURE_HEIGHT,
	GVS_TEXTURE_NORMAL,
	GVS_TEXTRUE_SHININESS,
	GVS_TEXTURE_DISPLACEMENT,
	GVS_TEXTURE_LIGHTMAP,
	GVS_TEXTURE_REFLECTION,
	GVS_TEXTURE_ROUGHNESS,
	GVS_TEXTURE_METALNESS,
	GVS_TEXTURE_UNKNOWN,

	// lean maps
	GVS_TEXTURE_LEAN1,
	GVS_TEXTURE_LEAN2,
	// shadow
	GVS_SHADOW_INDEX,

	// effect mask
	GVS_EFFECT_MASK
};


class PerObjectUniformGetter {
public:
	PerObjectUniformGetter() {
		InitStringEnumMap();
	}

	void GetAndSetUniforms(
        std::unordered_map<PerObjectUniform, std::string>& perObjUniforms,
        Pass* pass,
        Entity* entity,
		Camera* camera,
        as_Material* mat,
        GpuProgram* program,
        int texture_index);

	Matrix4 GetWorldMatrix(Entity* entity);
	Matrix4 GetViewMatrix(Camera* camera);
	Matrix4 GetProjectionMatrix(Camera* camera);
	Vector3 GetCameraPosition(Camera* camera);
	Vector4 GetMaterialValue(as_Material* mat, PerObjectUniform sem);
	Texture* GetMaterialTexture(as_Material* mat, PerObjectUniform sem);

    std::map<std::string, PerObjectUniform> m_StringSemanticMap;
	void InitStringEnumMap();
};
