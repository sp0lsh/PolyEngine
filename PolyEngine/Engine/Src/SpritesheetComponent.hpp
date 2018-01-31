#pragma once

#include "ComponentBase.hpp"
#include "RenderingSystem.hpp"

namespace Poly {

	class ENGINE_DLLEXPORT SpritesheetComponent : public ComponentBase
	{
	public:

		struct ENGINE_DLLEXPORT Settings
		{
			String SpritePath;
			eResourceSource Source = eResourceSource::GAME;
			Vector2f SubImages = Vector2f(4.0, 4.0);
			Color Color = Color::WHITE;
			float StartFrame = 0.0f;
			float Speed = 1.0f;
			float IsRandom = 0.0f;
		};

		friend void RenderingSystem::RenderingPhase(World*);
	public:
		SpritesheetComponent(const Settings& settings);
		virtual ~SpritesheetComponent();

		const TextureResource* GetSpritesheet() const { return Spritesheet; }
		const Settings& GetSettings() const { return settings; }

	private:
		Settings settings;
		TextureResource* Spritesheet; 
	};

	REGISTER_COMPONENT(ComponentsIDGroup, SpritesheetComponent)
}