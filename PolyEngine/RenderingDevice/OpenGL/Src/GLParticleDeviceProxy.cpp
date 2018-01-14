#include "GLMeshDeviceProxy.hpp"
#include "GLUtils.hpp"
#include "GLParticleDeviceProxy.hpp"

using namespace Poly;

//---------------------------------------------------------------
GLParticleDeviceProxy::GLParticleDeviceProxy()
{
	instanceVBO = 0;
}

//---------------------------------------------------------------
GLParticleDeviceProxy::~GLParticleDeviceProxy()
{
	glDeleteBuffers(1, &instanceVBO);

	// if(VAO)
	// 	glDeleteVertexArrays(1, &VAO);
}

//---------------------------------------------------------------
// void GLParticleDeviceProxy::SetContent(const Mesh& mesh)
// {
	// if (!VAO)
	// {
	// 	glGenVertexArrays(1, &VAO);
	// 	if (!VAO)
	// 		throw RenderingDeviceProxyCreationFailedException();
	// }
	// 	
	// glBindVertexArray(VAO);
	// 
	// ASSERTE(mesh.HasVertices() && mesh.HasIndicies(), "Meshes that does not contain vertices and faces are not supported yet!");
	// 
	// if (mesh.HasVertices()) {
	// 	EnsureVBOCreated(eBufferType::VERTEX_BUFFER);
	// 	glBindBuffer(GL_ARRAY_BUFFER, VBO[eBufferType::VERTEX_BUFFER]);
	// 	glBufferData(GL_ARRAY_BUFFER, mesh.GetPositions().GetSize() * sizeof(Mesh::Vector3D), mesh.GetPositions().GetData(), GL_STATIC_DRAW);
	// 	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	// 	glEnableVertexAttribArray(0);
	// 	CHECK_GL_ERR();
	// }
	// 
	// if (mesh.HasTextCoords()) {
	// 	EnsureVBOCreated(eBufferType::TEXCOORD_BUFFER);
	// 	glBindBuffer(GL_ARRAY_BUFFER, VBO[eBufferType::TEXCOORD_BUFFER]);
	// 	glBufferData(GL_ARRAY_BUFFER, mesh.GetTextCoords().GetSize() * sizeof(Mesh::TextCoord), mesh.GetTextCoords().GetData(), GL_STATIC_DRAW);
	// 	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	// 	glEnableVertexAttribArray(1);
	// 	CHECK_GL_ERR();
	// }
	// 
	// if (mesh.HasNormals()) {
	// 	EnsureVBOCreated(eBufferType::NORMAL_BUFFER);
	// 	glBindBuffer(GL_ARRAY_BUFFER, VBO[eBufferType::NORMAL_BUFFER]);
	// 	glBufferData(GL_ARRAY_BUFFER, mesh.GetNormals().GetSize() * sizeof(Mesh::Vector3D), mesh.GetNormals().GetData(), GL_STATIC_DRAW);
	// 	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	// 	glEnableVertexAttribArray(2);
	// 	CHECK_GL_ERR();
	// }
	// 
	// if (mesh.HasIndicies())
	// {
	// 	EnsureVBOCreated(eBufferType::INDEX_BUFFER);
	// 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[eBufferType::INDEX_BUFFER]);
	// 	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.GetIndicies().GetSize() * sizeof(GLuint), mesh.GetIndicies().GetData(), GL_STATIC_DRAW);
	// 
	// 	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	// 	glEnableVertexAttribArray(3);
	// 	CHECK_GL_ERR();
	// }
	// 
	// glBindBuffer(GL_ARRAY_BUFFER, 0);
	// glBindVertexArray(0);
// }

//---------------------------------------------------------------
// void GLParticleDeviceProxy::EnsureVBOCreated(eBufferType type)
// {
// 	// if (!VBO[type])
// 	// {
// 	// 	glGenBuffers(1, &VBO[type]);
// 	// 	if (!VBO[type])
// 	// 		throw RenderingDeviceProxyCreationFailedException();
// 	// }
// }

void GLParticleDeviceProxy::SetContent(const Dynarray<float>& instances)
{
	gConsole.LogInfo("GLParticleDeviceProxy::SetContent size: {}, instances: {}",
		instances.GetSize(), instances.GetSize()/16);

	instanceTransform.Resize(instances.GetSize());

	if (instanceVBO == 0)
	{
		glGenBuffers(1, &instanceVBO);
	}

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
