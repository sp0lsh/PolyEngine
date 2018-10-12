#pragma once

#include <Defines.hpp>
#include <ECS/ComponentBase.hpp>
#include <Rendering/MeshRenderingComponent.hpp>
#include <Rendering/RenderingSystem.hpp>
#include <Resources/MeshResource.hpp>

namespace Poly {
	
	class ENGINE_DLLEXPORT OrthoMeshRenderingComponent : public ComponentBase
	{
	public:
		RTTI_DECLARE_COMPONENT(::Poly::OrthoMeshRenderingComponent) { NO_RTTI_PROPERTY(); }

		OrthoMeshRenderingComponent(const String& meshPath, eResourceSource source);
		virtual ~OrthoMeshRenderingComponent();

		const MeshResource* GetMesh() const { return Mesh; }
		const Material& GetMaterial(size_t i) const { return Materials[i]; }
		Material& GetMaterial(size_t i) { return Materials[i]; }
		bool GetIsWireframe() const { return IsWireframe; }
		eShadingMode GetShadingModel() const { return ShadingMode; }
		eBlendingMode GetBlendingMode() const { return BlendingMode; }
		
		void SetMaterial(size_t i, const Material& value) { Materials[i] = value; }
		void SetIsWireframe(bool value) { IsWireframe = value; }
		void SetShadingModel(eShadingMode value) { ShadingMode = value; }
		void SetBlendingMode(eBlendingMode value) { BlendingMode = value; }

		Optional<AABox> GetBoundingBox(eEntityBoundingChannel channel) override;

	private:
		MeshResource* Mesh = nullptr;
		Dynarray<Material> Materials;
		eShadingMode ShadingMode = eShadingMode::PBR;
		eBlendingMode BlendingMode = eBlendingMode::OPAUQE;
		bool IsWireframe = false;
	};

	REGISTER_COMPONENT(ComponentsIDGroup, OrthoMeshRenderingComponent)
}