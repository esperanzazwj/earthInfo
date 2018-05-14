#include "RenderStrategy.h"
#include "RenderSystem.h"
#include "OMesh.h"
#include "PreDefine.h"
#include "GpuProgram.h"
#include "Entity.h"
#include "RenderTarget.h"
#include "Camera.h"
#include "Geometry.h"
#include "OpenglDriver/EGLUtil.h"
#include "ResourceManager.h"
#include "animation_helper.h"
#include "pass.h"
#include "TextureManager.h"
#include "PerObjectUniform.h"
#include "UniformSetter.h"
//vector<RenderQueue> SplitRenderQueue(RenderQueue& queue, int maxNum = 0) {
//	vector<RenderQueue> outqueue;
//	SortByMeshName(queue);
//
//	string lastname;
//	int count = 0;
//	for (int i = 0; i < queue.size(); i++) {
//		if (queue[i].asMesh == NULL) continue;
//		count++;
//		if (outqueue.size() == 0 || queue[i].asMesh->name != lastname || count == maxNum) {
//			outqueue.push_back(RenderQueue());
//			count = 0;
//		}
//		lastname = queue[i].asMesh->name;
//		int l = outqueue.size();
//		outqueue[l - 1].push_back(queue[i]);
//	}
//
//
//	return outqueue;
//}
//void CollectWorldMatrixs(RenderQueue& queue, vector<Matrix4>& out) {
//	out = vector<Matrix4>(queue.size());
//	for (int i = 0; i < queue.size(); i++) {
//		if (queue[i].entity != NULL) {
//			out[i] = queue[i].entity->getParent()->getWorldMatrix();
//		}
//	}
//}
//
//void RenderStrategy::RenderPass_old(Camera* camera, RenderQueue & renderqueue, as_Pass* pass, RenderTarget * rt)
//{
//	assert(rt != NULL);
//	assert(pass != NULL);
//	auto rhi = GlobalResourceManager::getInstance().m_RenderSystem;
//
//	GlobalVariablePool* gp = GlobalVariablePool::GetSingletonPtr();
//	Entity * entity = NULL;
//	auto program = pass->mProgram;
//	rhi->BindGpuProgram(program);
//	rhi->SetRenderTarget(rt);
//
//	if (pass->mClearState.mclearcolorbuffer | pass->mClearState.mcleardepthbuffer | pass->mClearState.mclearstencilbuffer) {
//		// to be fixed
//		if (pass->name == "gbuffer pass") {
//			const unsigned clear_color_value[4] = { 0 };
//			float colorf[4] = { 0,0,0,0 };
//			const float clear_depth_value = 1.f;
//			glClearBufferuiv(GL_COLOR, 3, clear_color_value);
//			glClearBufferfv(GL_COLOR, 0, colorf);
//			glClearBufferfv(GL_COLOR, 1, colorf);
//			glClearBufferfv(GL_COLOR, 2, colorf);
//			glClearBufferfv(GL_DEPTH, 0, &clear_depth_value);
//			GL_CHECK();
//		}
//		else
//		{
//			Vector4 c = pass->mClearState.mClearColor;
//			rhi->SetClearColor(c.x, c.y, c.z, c.w);
//			rhi->ClearBuffer(pass->mClearState);
//		}
//	}
//
//	rhi->SetViewport(0, 0, rt->getWidth(), rt->getHeight());
//	rhi->setBlendState(pass->mBlendState);
//	rhi->setDepthStencilState(pass->mDepthStencilState);
//	rhi->setRasterState(pass->mRasterState);
//
//
//	if (camera != NULL)
//		gp->SetCamera(camera);
//
//
//	if (pass->UseInstance) {
//		auto splitdQueues = SplitRenderQueue(renderqueue, pass->InstanceBatchNum);
//		for (auto& queue : splitdQueues) {
//
//			vector<Matrix4> worldMatrixs;
//			CollectWorldMatrixs(queue, worldMatrixs);
//			auto x = queue[0];
//			if (x.asMesh != NULL)
//				gp->SetMaterial(x.asMesh->material);
//			program->setProgramConstantData("InstanceMatrix", worldMatrixs.data(), "mat4",
//				worldMatrixs.size() * sizeof(Matrix4));
//			program->texture_count = 0;
//			program->updateProgramData(program->m_ProgramData);
//			program->UpdateGlobalVariable(pass->mUniformMap);
//
//			auto& currentGeo = x.asMesh->renderable;
//			if (currentGeo == NULL) {
//				auto g = GlobalResourceManager::getInstance().m_GeometryFactory;
//				auto& grm = GlobalResourceManager::getInstance();
//				//auto eg = static_cast<GLGeometryFactory*>(g);
//				currentGeo = g->create(x.asMesh->geometry, pass->mInputLayout);
//			}
//			rhi->DrawGeometryInstance(currentGeo, queue.size());
//		}
//	}
//	else {
//		program->texture_count = 0;
//		program->updateProgramData(program->m_ProgramData);
//		for (auto& x : renderqueue) {
//			if (x.asMesh == NULL) continue;
//			if (x.entity != NULL) {
//
//				gp->SetWorldMatrix(x.entity->getParent()->getWorldMatrix());
//			}
//
//			//gp->setMatrices(temp_matrices);
//			if (x.asMesh != NULL)
//				gp->SetMaterial(x.asMesh->material);
//
//			program->UpdateGlobalVariable(pass->mUniformMap);
//
//			auto& currentGeo = x.asMesh->renderable;
//			if (currentGeo == NULL) {
//				auto g = GlobalResourceManager::getInstance().m_GeometryFactory;
//				auto& grm = GlobalResourceManager::getInstance();
//				//auto eg = static_cast<GLGeometryFactory*>(g);
//				currentGeo = g->create(x.asMesh->geometry, pass->mInputLayout);
//			}
//			rhi->DrawGeometry(currentGeo);
//		}
//
//	}
//}
//{

void RenderStrategy::RenderPass(Camera* camera, RenderQueue & renderqueue, Pass* pass, RenderTarget * rt)
{

	auto rhi = GlobalResourceManager::getInstance().m_RenderSystem;
	auto uniformsetter= GlobalResourceManager::getInstance().m_UniformSetter;
	auto program = pass->as_pass->mProgram;
	
	SetPassState(camera, pass, rt);
	
	//rhi->SetRenderTarget(rt);
	rhi->BindGpuProgram(program);

	int texture_count = 1;
	for (auto& it : pass->mPerFrameUniform) {
		const UniformData & ud = it.second;
		string name = it.first;
		if (ud.type == UT_Texture)
		{	
			uniformsetter->SetProgramTexture(name, program, ud.tex, texture_count);
			uniformsetter->SetTexureSampler(name, pass, ud.tex, texture_count);
			texture_count++;
		}
		else if (ud.type == UT_Value)
		{
			int count = ud.size / UniformData::GetTypeSize(ud.datatype);
			rhi->SetUniform(name, program, ud.datatype, count, (void*)&ud.data[0]);
		}
	}
	
	for (auto& x : renderqueue) {
		if (x.asMesh == NULL) continue;
		auto uniformGetter = GlobalResourceManager::getInstance().m_PerObjectUniformGetter;
		uniformGetter->GetAndSetUniforms(pass->mPerObjectUniform, pass, x.entity, camera, x.asMesh->material, program, texture_count);
		auto& currentGeo = x.asMesh->renderable;
		if (currentGeo == NULL) {
			auto g = GlobalResourceManager::getInstance().m_GeometryFactory;
			currentGeo = g->create(x.asMesh->geometry, pass->mInputLayout);
		}
		rhi->DrawGeometry(currentGeo);
	}
}

void RenderStrategy::RenderPassMini(Camera* /*camera*/, RenderQueue & /*renderqueue*/, as_Pass* /*pass*/, RenderTarget * /*rt*/)
{

}

void RenderStrategy::SetPassState(Camera* /*camera*/, Pass* pass, RenderTarget * rt)
{
	assert(rt != NULL);
	assert(pass != NULL);
	auto rhi = GlobalResourceManager::getInstance().m_RenderSystem;
	
	rhi->SetRenderTarget(rt);
	if (pass->mClearState.mclearcolorbuffer | pass->mClearState.mcleardepthbuffer | pass->mClearState.mclearstencilbuffer) {
		// to be fixed
		if (pass->name == "gbuffer pass") {
			const unsigned clear_color_value[4] = { 0 };
			float colorf[4] = { 0,0,0,0 };
			glClearBufferuiv(GL_COLOR, 3, clear_color_value);
			glClearBufferfv(GL_COLOR, 0, colorf);
			glClearBufferfv(GL_COLOR, 1, colorf);
			glClearBufferfv(GL_COLOR, 2, colorf);
			glClearBufferfv(GL_DEPTH, 0, &(pass->mClearState.mClearDepth));
			GL_CHECK();
		}
		else
		{
			if (pass->mClearState.clearFlag)
			{
				Vector4 c = pass->mClearState.mClearColor;
				rhi->SetClearColor(c.x, c.y, c.z, c.w);
				rhi->ClearBuffer(pass->mClearState);
			}
		}
	}

	if(!pass->mRasterState.viewPortFlag)
		rhi->SetViewport(0, 0, (float)rt->getWidth(), (float)rt->getHeight());

	rhi->setBlendState(pass->mBlendState);
	rhi->setDepthStencilState(pass->mDepthStencilState);
	rhi->setRasterState(pass->mRasterState);


}

