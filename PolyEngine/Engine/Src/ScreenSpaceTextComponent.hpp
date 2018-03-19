#pragma once

#include "ComponentBase.hpp"
#include "RenderingSystem.hpp"

#include "Text2D.hpp"

namespace Poly {

	// This is temporary class
	class ENGINE_DLLEXPORT ScreenSpaceTextComponent : public ComponentBase
	{
	public:
		ScreenSpaceTextComponent(const Vector2i& pos, const String& fontName, eResourceSource source, size_t fontSize, const String& text = "", const Color& fontColor = Color(1, 1, 1))
			: Text(fontName, source, fontSize, text, fontColor), ScreenPosition(pos) {}

		Text2D& GetText() { return Text; }
		const Text2D& GetText() const { return Text; }

		void SetText(const String& text) { Text.SetText(text); }

		void SetScreenPosition(const Vector2i& pos) { ScreenPosition = pos; }
		const Vector2i& GetScreenPosition() const { return ScreenPosition; }
	private:
		Text2D Text;
		Vector2i ScreenPosition;
	};

	REGISTER_COMPONENT(ComponentsIDGroup, ScreenSpaceTextComponent)
}