#pragma once

#include <Defines.hpp>
#include <Rendering/Viewport.hpp>
#include <Rendering/Lighting/LightSourceComponent.hpp>
#include <Rendering/RenderingSettingsComponent.hpp>

// TODO: inherit from BaseRenderPass - make multipass RenderPass

namespace Poly {

	class GLRenderingDevice;

	struct ScreenSize;
	class Scene;
	class AARect;
	class CameraComponent;
	class MeshRenderingComponent;

	struct SceneView : public BaseObject<> {
		SceneView(Scene* w, const Viewport& v)
			: SceneData(w), ViewportData(v), Rect(v.GetRect()), CameraCmp(v.GetCamera())
		{
			SettingsCmp = CameraCmp->GetSibling<RenderingSettingsComponent>();
		};

		Scene* SceneData;
		const Viewport& ViewportData;
		const AARect& Rect;
		const CameraComponent* CameraCmp;
		const RenderingSettingsComponent* SettingsCmp;
		
		Dynarray<const MeshRenderingComponent*> DirShadowOpaqueQueue;
		Dynarray<const MeshRenderingComponent*> OpaqueQueue;
		Dynarray<const MeshRenderingComponent*> TranslucentQueue;

		AABox DirShadowAABBInLS;
		Dynarray<DirectionalLightComponent*> DirectionalLights;
		Dynarray<const PointLightComponent*> PointLights;
	};

	class IRendererInterface : public BaseObject<>
	{
	public:
		IRendererInterface(GLRenderingDevice* RenderingDeviceInterface);

		virtual void Init() = 0;		
		virtual void Resize(const ScreenSize& size) = 0;
		virtual void Render(const SceneView& sceneView) = 0;
		virtual void Deinit() = 0;

	protected:
		GLRenderingDevice* RDI;
	};
}