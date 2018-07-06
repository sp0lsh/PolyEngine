#pragma once

#include "ECS/ComponentBase.hpp"
#include "RenderingSystem.hpp"
#include "Resources/MeshResource.hpp"

namespace Poly {

	enum class eShadingMode
	{
		NONE,
		UNLIT,
		PBR,
		_COUNT
	};

	enum class eBlendingMode
	{
		NONE,
		OPAUQE,
		TRANSLUCENT,
		_COUNT
	};

	struct ENGINE_DLLEXPORT Material
	{
		Material()
			: Emissive(0.0f, 0.0f, 0.0f), Albedo(1.0f, 1.0f, 1.0f), Roughness(1.0f), Metallic(1.0f), OpacityMaskThreshold(0.5f)
		{}

		Material(const Color& emissive, const Color& albedo, float roughness, float metallic, float opacityMaskThreshold)
			: Emissive(emissive), Albedo(albedo), Roughness(roughness), Metallic(metallic), OpacityMaskThreshold(opacityMaskThreshold)
		{}

		Color Emissive;
		Color Albedo;
		float Roughness;
		float Metallic;
		float OpacityMaskThreshold;
	};

	class ENGINE_DLLEXPORT MeshRenderingComponent : public ComponentBase
	{
	public:
		MeshRenderingComponent(const String& meshPath, eResourceSource source);
		virtual ~MeshRenderingComponent();

		const MeshResource* GetMesh() const { return Mesh; }
		const Material& GetMaterial(int i) const { return Materials[i]; }
		bool GetIsWireframe() const { return IsWireframe; }
		eShadingMode GetShadingModel() const { return ShadingMode; }
		eBlendingMode GetBlendingMode() const { return BlendingMode; }
		
		void SetMaterial(size_t i, const Material& value) { Materials[i] = value; }
		void SetIsWireframe(bool value) { IsWireframe = value; }
		void SetShadingModel(eShadingMode value) { ShadingMode = value; }
		void SetBlendingMode(eBlendingMode value) { BlendingMode = value; }

	private:
		MeshResource* Mesh = nullptr;
		Dynarray<Material> Materials;
		eShadingMode ShadingMode = eShadingMode::PBR;
		eBlendingMode BlendingMode = eBlendingMode::OPAUQE;
		bool IsWireframe = false;
	};

	REGISTER_COMPONENT(ComponentsIDGroup, MeshRenderingComponent)
}