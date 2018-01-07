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
	// translation = {}; // init array with zeros
	int index = 0;
	float offset = 0.1f;
	for (int y = -5; y < 5; ++y)
	{
		for (int x = -5; x < 5; ++x)
		{
			translation[index++] = (float)x / 5.0f + offset;
			translation[index++] = (float)y / 5.0f + offset;
		}
	}

	GetProgram().RegisterUniform("float", "uTime");

	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 100, &translation[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	float quadVertices[] = {
		// positions			// colors
		-1.0f,  1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,		0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,		0.0f, 0.0f, 1.0f,
					  
		-1.0f,  1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,		0.0f, 1.0f, 0.0f,
	 	 1.0f, -1.0f, 0.0f,		0.0f, 1.0f, 1.0f
	};
	
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	// also set instance data
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.

}

void InstancedMeshRenderingPass::OnRun(World* world, const CameraComponent* camera, const AARect& /*rect*/, ePassType passType = ePassType::GLOBAL)
{
	gConsole.LogInfo("InstancedMeshRenderingPass::OnRun");
	
	float Time = (float)TimeSystem::GetTimerElapsedTime(world, eEngineTimer::GAMEPLAY);

	GetProgram().BindProgram();
	const Matrix& mvp = camera->GetMVP();

	GetProgram().BindProgram();
	GetProgram().SetUniform("uTime", Time);

	glBindVertexArray(quadVAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100); // 100 triangles of 6 vertices each
	glBindVertexArray(0);

	// glBindVertexArray(VAO);
	// glDrawArrays(GL_TRIANGLES, 0, 6);
	// glBindVertexArray(0);
}