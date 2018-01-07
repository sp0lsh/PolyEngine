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
	// glEnableVertexAttribArray(2);
	// glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
	// glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	// glBindBuffer(GL_ARRAY_BUFFER, 0);
	// glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.

	// gConsole.LogInfo("InstancedMeshRenderingPass::Ctor sizeof(Matrix): {}, sizeof(GLfloat): {}", sizeof(Matrix), sizeof(GLfloat));

	// fill array with zeros
	for (int i = 0; i < 16 * 100; ++i)
	{
		translation[i] = 0.0f;
	}

	int index = 0;
	float offset = 0.1f;
	for (int y = -0; y < 10; ++y)
	{
		for (int x = -0; x < 10; ++x)
		{
			// identity
			translation[index + 0] = 1.0f;
			translation[index + 5] = 1.0f;
			translation[index + 9] = 1.0f;
			translation[index + 15] = 1.0f;
			// translation
			translation[index + 3] = (float)x / 5.0f + offset;
			translation[index + 7] = (float)y / 5.0f + offset;
			index += 16;
		}
	}

	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16 * 100, &translation[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// int pos = glGetAttribLocation((GLint)GetProgram().GetProgramHandle(), "aOffset");
	int pos = 2;
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