#include "InstancedMeshRenderingPass.hpp"

#include "GLMeshDeviceProxy.hpp"
#include "GLTextureDeviceProxy.hpp"

#include <World.hpp>
#include <TimeSystem.hpp>
#include <CameraComponent.hpp>
#include <TransformComponent.hpp>
#include <MeshRenderingComponent.hpp>
#include <MovementSystem.hpp>

using namespace Poly;

InstancedMeshRenderingPass::InstancedMeshRenderingPass(const PostprocessQuad* quad)
	: RenderingPassBase("Shaders/instancedVert.shader", "Shaders/instancedFrag.shader"), Quad(quad)
{
	GetProgram().RegisterUniform("float", "uTime");
	GetProgram().RegisterUniform("mat4", "uMVP");

	float quadVertices[] = {
		// positions			// colors
		-1.0f,  1.0f, 0.0f,		1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,		0.0f, 1.0f,
		 1.0f,  1.0f, 0.0f,		0.0f, 0.0f,
					  
		-1.0f,  1.0f, 0.0f,		1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,		0.0f, 1.0f,
	 	 1.0f, -1.0f, 0.0f,		0.0f, 1.0f
	};
	
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	
	// gConsole.LogInfo("InstancedMeshRenderingPass::Ctor sizeof(Matrix): {}, sizeof(GLfloat): {}", sizeof(Matrix), sizeof(GLfloat));

	// fill array with zeros
	for (int i = 0; i < 16 * instancesLen; ++i)
	{
		instanceTransform[i] = 0.0f;
	}

	srand(42);

	int index = 0;
	for (int i = 0; i < instancesLen; ++i)
	{
		// must be transposed
		// identity
		instanceTransform[index + 0] = 1.0f;
		instanceTransform[index + 5] = 1.0f;
		instanceTransform[index + 10] = 1.0f;
		instanceTransform[index + 15] = 1.0f;
		// translation
		instanceTransform[index + 12] = Random(-1.0, 1.0);
		instanceTransform[index + 13] = Random(-1.0, 1.0);
		// instanceTransform[index + 14] = Random(-1.0, 1.0);
		index += 16;
	}

	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16 * instancesLen, &instanceTransform[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// http://sol.gfxile.net/instancing.html
	// int pos = glGetAttribLocation((GLint)GetProgram().GetProgramHandle(), "aOffset");
	int pos = 3;
	int pos1 = pos + 0;
	int pos2 = pos + 1;
	int pos3 = pos + 2;
	int pos4 = pos + 3;
	glEnableVertexAttribArray(pos1);
	glEnableVertexAttribArray(pos2);
	glEnableVertexAttribArray(pos3);
	glEnableVertexAttribArray(pos4);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glVertexAttribPointer(pos1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, (void*)(0));
	glVertexAttribPointer(pos2, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, (void*)(sizeof(float) * 4));
	glVertexAttribPointer(pos3, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, (void*)(sizeof(float) * 8));
	glVertexAttribPointer(pos4, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4 * 4, (void*)(sizeof(float) * 12));
	glVertexAttribDivisor(pos1, 1);
	glVertexAttribDivisor(pos2, 1);
	glVertexAttribDivisor(pos3, 1);
	glVertexAttribDivisor(pos4, 1);
}

void InstancedMeshRenderingPass::OnRun(World* world, const CameraComponent* camera, const AARect& /*rect*/, ePassType passType = ePassType::GLOBAL)
{
	// gConsole.LogInfo("InstancedMeshRenderingPass::OnRun");
	float Time = (float)TimeSystem::GetTimerElapsedTime(world, eEngineTimer::GAMEPLAY);

	GetProgram().BindProgram();
	const Matrix& mvp = camera->GetMVP();
	// Matrix translation;
	// translation.SetTranslation(Vector(0.0f, 5.0f, 5.0));
	// const TransformComponent* transCmp = camera->GetSibling<TransformComponent>();
	// const Matrix& objTransform = transCmp->GetGlobalTransformationMatrix();
	// Matrix inst = Matrix(&instanceTransform[0]);

	// gConsole.LogInfo("InstancedMeshRenderingPass::OnRun MVP: {}, InstTrans: {}", objTransform, inst);

	// GetProgram().BindProgram();
	// GetProgram().SetUniform("uMVP", mvp*translation);
	// 
	// glBindVertexArray(quadVAO);
	// glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instancesLen); // 100 triangles of 6 vertices each
	// glBindVertexArray(0);

	// // Render meshes
	// for (auto componentsTuple : world->IterateComponents<MeshRenderingComponent, TransformComponent>())
	// {
	// 	const MeshRenderingComponent* meshCmp = std::get<MeshRenderingComponent*>(componentsTuple);
	// 	for (const Mesh* subMesh : meshCmp->GetMesh()->GetMeshes())
	// 	{
	// 		// const GLMeshDeviceProxy* meshProxy = static_cast<const GLMeshDeviceProxy*>(subMesh->GetMeshProxy());
	// 		if (subMesh->HasInstances()) {
	// 			gConsole.LogInfo("InstancedMeshRenderingPass::OnRun instances: {}", subMesh->GetInstances().GetSize());
	// 		}
	// 	}
	// }
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Render meshes
	for (auto componentsTuple : world->IterateComponents<MeshRenderingComponent, TransformComponent>())
	{
		const MeshRenderingComponent* meshCmp = std::get<MeshRenderingComponent*>(componentsTuple);
		const TransformComponent* transCmp = std::get<TransformComponent*>(componentsTuple);

//		Vector objPos = transCmp->GetGlobalTranslation();

		const Matrix& objTransform = transCmp->GetGlobalTransformationMatrix();
		Matrix screenTransform = mvp * objTransform;
		// GetProgram().SetUniform("uTransform", objTransform);
		GetProgram().SetUniform("uTime", Time);
		GetProgram().SetUniform("uMVP", screenTransform);


		for (const Mesh* subMesh : meshCmp->GetMesh()->GetMeshes())
		{
			if (subMesh->HasInstances()) {
				gConsole.LogInfo("InstancedMeshRenderingPass::OnRun instances: {}", subMesh->GetInstancesCount());
				const GLMeshDeviceProxy* meshProxy = static_cast<const GLMeshDeviceProxy*>(subMesh->GetMeshProxy());
				glBindVertexArray(meshProxy->GetVAO());

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, FallbackWhiteTexture);

				glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)subMesh->GetVertexCount(), (GLsizei)subMesh->GetInstancesCount());
				// glDrawElements(GL_TRIANGLES, (GLsizei)subMesh->GetTriangleCount() * 3, GL_UNSIGNED_INT, NULL);
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindVertexArray(0);
			}
		}
	}
	
	glEnable(GL_CULL_FACE);
}

float InstancedMeshRenderingPass::Random() const
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

float InstancedMeshRenderingPass::Random(float min, float max) const
{
	float rnd = Random();
	return Lerp(min, max, rnd);
}
