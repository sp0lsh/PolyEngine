#include <EnginePCH.hpp>

#include <Resources/TextureResource.hpp>
#include <Resources/ResourceManager.hpp>
#include <Rendering/IRenderingDevice.hpp>
#include <ECS/Scene.hpp>

using namespace Poly;

static int GetDesiredChannel(eTextureUsageType usage) noexcept
{
	switch (usage)
	{
	case eTextureUsageType::ALBEDO:
	case eTextureUsageType::RENDER_TARGET:
		return 0;
	case eTextureUsageType::EMISSIVE:
	case eTextureUsageType::AMBIENT_OCCLUSION:
	case eTextureUsageType::METALLIC:
	case eTextureUsageType::ROUGHNESS:
	case eTextureUsageType::ORM:
	case eTextureUsageType::NORMAL:
	case eTextureUsageType::HDR:
	case eTextureUsageType::FONT:
		return 3;
	default:
		ASSERTE(false, "Uknown eTextureUsageType!");
	}
	return 0;
}

TextureResource::TextureResource(const String& path, eTextureUsageType usage)
{
	gConsole.LogInfo("TextureResource::TextureResource path: {} usage: {}", path, (int)usage);

	int desiredChannels = GetDesiredChannel(usage);

	if (usage == eTextureUsageType::HDR) 
	{
		float* Image = LoadImageHDR(path.GetCStr(), &Width, &Height, &Channels);
	
		gConsole.LogInfo("TextureResource::TextureResource loaded width: {}, height: {}, channels: {}", Width, Height, Channels);
		
		if (Image)
		{
			TextureProxy = gEngine->GetRenderingDevice()->CreateTexture(Width, Height, Channels, usage);
			TextureProxy->SetContentHDR(Image);
			FreeImageHDR(Image);
		}
	}
	else
	{
		unsigned char* Image = LoadImage(path.GetCStr(), &Width, &Height, &Channels);

		gConsole.LogInfo("TextureResource::TextureResource loaded width: {}, height: {}, channels: {}",
			Width, Height, Channels, desiredChannels);
		
		if (Image)
		{
			TextureProxy = gEngine->GetRenderingDevice()->CreateTexture(Width, Height, Channels, usage);
			TextureProxy->SetContent(Image);
			FreeImage(Image);
		}
	}
}

TextureResource::~TextureResource()
{
}
