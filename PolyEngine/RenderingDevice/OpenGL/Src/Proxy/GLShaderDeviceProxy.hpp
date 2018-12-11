#pragma once

#include <Defines.hpp>
#include <Common/GLUtils.hpp>

namespace Poly
{
	class GLShaderDeviceProxy : public IShaderDeviceProxy
	{
	public:
		GLShaderDeviceProxy();
		virtual ~GLShaderDeviceProxy();
		
		void SetContent() override;
		unsigned int GetResourceID() const override { return 0; };
	};
}