#pragma once

#include <Defines.hpp>
#include <Resources/ResourceBase.hpp>
#include <Rendering/IRenderingDevice.hpp>

namespace Poly 
{
	class IShaderDeviceProxy;

	enum class eBlendingMode
	{
		NONE,
		OPAUQE,
		TRANSLUCENT,
		_COUNT
	};

	REGISTER_ENUM_NAMES(eBlendingMode, "NONE", "OPAQUE", "TRANSLUCENT");

	// class ENGINE_DLLEXPORT MaterialAsset : public RTTIBase
	// {
	// 	RTTI_DECLARE_TYPE_DERIVED(MaterialAsset, RTTIBase)
	// 	{
	// 		RTTI_PROPERTY(Blending,				"Blending",				RTTI::ePropertyFlag::NONE);
	// 		RTTI_PROPERTY(VertexShaderPath,		"VertexShaderPath",		RTTI::ePropertyFlag::NONE);
	// 		RTTI_PROPERTY(FrangmentShaderPath,	"FrangmentShaderPath",	RTTI::ePropertyFlag::NONE);
	// 	}
	// 
	// 	MaterialAsset()
	// 	{
	// 	}
	// 
	// 	eBlendingMode Blending;
	// 	String VertexShaderPath;
	// 	String FrangmentShaderPath;
	// };

	class ENGINE_DLLEXPORT ShaderResource : public ResourceBase
	{
	public:
		ShaderResource(const String& absolutePath);
		~ShaderResource() override;

		void Compile();
		
		const IShaderDeviceProxy* GetShaderProxy() const { return ShaderProxy.get(); }

	private:
		
		// MaterialAsset DefaultMaterial;
		std::unique_ptr<IShaderDeviceProxy> ShaderProxy;
	};
}

