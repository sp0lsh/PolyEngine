#pragma once

#include <IRenderingDevice.hpp>
#include <EnumUtils.hpp>
#include <Matrix.hpp>
#include "GLUtils.hpp"

namespace Poly
{
	class GLParticleDeviceProxy : public IParticleDeviceProxy
	{
	private:
		// enum class eBufferType {
		// 	VERTEX_BUFFER,
		// 	TEXCOORD_BUFFER,
		// 	NORMAL_BUFFER,
		// 	INDEX_BUFFER,
		// 	_COUNT
		// };

	public:
		GLParticleDeviceProxy();
		virtual ~GLParticleDeviceProxy();

		void SetContent(const Dynarray<float>& instances);

		// void SetContent(const Mesh& mesh);
		// 
		// GLuint GetVAO() const { return VAO; }

	private:

		unsigned int instanceVBO;

		static const int instancesLen = 0;

		Dynarray<float> instanceTransform;

		// void EnsureVBOCreated(eBufferType type);
		// 
		// GLuint VAO = 0;
		// EnumArray<GLuint, eBufferType> VBO;
		// 
		// friend class GLRenderingDevice;
	};
}
