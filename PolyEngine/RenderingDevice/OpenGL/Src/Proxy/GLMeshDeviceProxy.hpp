#pragma once

#include <Rendering/IRenderingDevice.hpp>
#include <Utils/EnumUtils.hpp>
#include "Common/GLUtils.hpp"

namespace Poly
{
	class GLMeshDeviceProxy : public IMeshDeviceProxy
	{
	private:
		enum class eBufferType {
			VERTEX_BUFFER,
			TEXCOORD_BUFFER,
			NORMAL_BUFFER,
			TANGENT_BUFFER,
			BITANGENT_BUFFER,
			INDEX_BUFFER,
			_COUNT
		};

	public:
		GLMeshDeviceProxy();
		virtual ~GLMeshDeviceProxy();

		void SetContent(const Mesh& mesh);
		unsigned int GetResourceID() const { return VAO; };

		GLuint GetVAO() const { return VAO; }

	private:
		void EnsureVBOCreated(eBufferType type);

		GLuint VAO = 0;
		EnumArray<GLuint, eBufferType> VBO;
	};
}