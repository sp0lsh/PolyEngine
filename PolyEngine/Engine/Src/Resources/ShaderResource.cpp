#include <EnginePCH.hpp>

#include <Resources/ShaderResource.hpp>
#include <Resources/ResourceManager.hpp>
#include <ECS/Scene.hpp>

using namespace Poly;

RTTI_DEFINE_TYPE(Poly::MaterialAsset)

ShaderResource::ShaderResource(const String& absolutePath)
{
	gConsole.LogInfo("ShaderResource::ShaderResource: path {}", absolutePath);
	Material.DeserializeFromFile(absolutePath);
	
	ShaderProxy = gEngine->GetRenderingDevice()->CreateShader();
	ShaderProxy->Compile(Material);
}

ShaderResource::~ShaderResource()
{
}

void ShaderResource::Compile()
{
	ShaderProxy->Compile(Material);
}
