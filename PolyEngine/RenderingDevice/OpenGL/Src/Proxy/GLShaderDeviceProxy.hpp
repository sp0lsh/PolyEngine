#pragma once

#include <Defines.hpp>
#include <Common/GLUtils.hpp>
#include <Resources/ShaderResource.hpp>

namespace Poly
{
	class GLShaderDeviceProxy : public IShaderDeviceProxy
	{
	public: // Inherited via IShaderDeviceProxy
		GLShaderDeviceProxy();
		virtual ~GLShaderDeviceProxy();
		virtual void Compile(MaterialAsset& materialAsset) override;
		unsigned int GetResourceID() const override { return 0; };
	};
}