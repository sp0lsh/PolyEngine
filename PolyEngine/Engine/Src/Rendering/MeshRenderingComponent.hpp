#pragma once

#include <Defines.hpp>
#include <ECS/ComponentBase.hpp>
#include <Rendering/RenderingSystem.hpp>
#include <Resources/ResourceManager.hpp>
#include <Resources/MeshResource.hpp>
#include <Resources/ShaderResource.hpp>

namespace Poly {

	enum class eShadingMode
	{
		NONE,
		UNLIT,
		PBR,
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
		RTTI_DECLARE_COMPONENT(::Poly::MeshRenderingComponent) { NO_RTTI_PROPERTY(); }

		MeshRenderingComponent(const String& meshPath, eResourceSource source);
		virtual ~MeshRenderingComponent();

		const MeshResource* GetMesh() const { return Mesh; }
		const Material& GetMaterial(int i) const { return Materials[i]; }
		const ShaderResource* GetShaderResource(int i) const { return ShaderResources[i]; }
		bool GetIsWireframe() const { return IsWireframe; }
		eShadingMode GetShadingModel() const { return ShadingMode; }
		eBlendingMode GetBlendingMode() const { return BlendingMode; }
		
		void SetMaterial(size_t i, const Material& value) { Materials[i] = value; }
		void SetShaderResource(size_t i, ShaderResource* value)
		{
			if (value->GetShaderProxy()->GetResourceID() == 0)
				value->Compile();
			ShaderResources[i] = value;
		}
		void SetIsWireframe(bool value) { IsWireframe = value; }
		void SetShadingModel(eShadingMode value) { ShadingMode = value; }
		void SetBlendingMode(eBlendingMode value) { BlendingMode = value; }

		Optional<AABox> GetBoundingBox(eEntityBoundingChannel channel) const override;

	private:
		MeshResource* Mesh = nullptr;
		Dynarray<Material> Materials;
		Dynarray<ShaderResource*> ShaderResources;
		eShadingMode ShadingMode = eShadingMode::PBR;
		eBlendingMode BlendingMode = eBlendingMode::OPAUQE;
		bool IsWireframe = false;
	};

	REGISTER_COMPONENT(ComponentsIDGroup, MeshRenderingComponent)
}