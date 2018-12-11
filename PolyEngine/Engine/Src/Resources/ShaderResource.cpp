#include <EnginePCH.hpp>

#include <Resources/ShaderResource.hpp>
#include <Resources/ResourceManager.hpp>
#include <ECS/Scene.hpp>

using namespace Poly;

ShaderResource::ShaderResource(const String& absolutePath)
{
	gConsole.LogInfo("ShaderResource::ShaderResource: path {}", absolutePath);
	// DefaultMaterial.DeserializeFromFile(absolutePath);
	
	ShaderProxy = gEngine->GetRenderingDevice()->CreateShader();
}

ShaderResource::~ShaderResource()
{
}

void ShaderResource::Compile()
{
	ShaderProxy->Compile();
}
