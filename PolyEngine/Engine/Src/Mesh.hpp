#pragma once

#include <BaseObject.hpp>
#include <Dynarray.hpp>
#include <Vector.hpp>
#include <Vector2f.hpp>
#include <Color.hpp>

typedef unsigned int GLuint;

namespace Poly
{
	class TextureResource;
	class IMeshDeviceProxy;

	class ENGINE_DLLEXPORT Mesh : public BaseObject<>
	{
	public:
		virtual ~Mesh();

		// struct ENGINE_DLLEXPORT Vector3D { float X = 0, Y = 0, Z = 0; };
		// struct ENGINE_DLLEXPORT TextCoord { float U = 0, V = 0; };
		struct ENGINE_DLLEXPORT Material
		{
			float SpecularIntensity;
			float SpecularPower;
			Color SpecularColor;
			TextureResource* DiffuseTexture;
			const TextureResource* GetDiffTexture() const { return DiffuseTexture; }
		};

		
		const Material& GetMaterial() const { return Mtl; }
		void SetMaterial(const Material& value);

		size_t GetVertexCount() const { return Positions.GetSize(); }
		size_t GetTriangleCount() const { return Indices.GetSize() / 3; }

		const Dynarray<Vector3f>& GetPositions() const { return Positions; }
		const Dynarray<Vector3f>& GetNormals() const { return Normals; }
		const Dynarray<Vector2f>& GetTextCoords() const { return TextCoords; }
		const Dynarray<uint32_t>& GetIndicies() const { return Indices; }

		void SetPositions(const Dynarray<Vector3f>& values) { Positions = values; }
		void SetNormals(const Dynarray<Vector3f>& values) { Normals = values; }
		void SetTextCoords(const Dynarray<Vector2f>& values) { TextCoords = values; }
		void SetIndicies(const Dynarray<uint32_t>& values) { Indices = values; }

		bool HasVertices() const { return Positions.GetSize() != 0; }
		bool HasNormals() const { return Normals.GetSize() != 0; }
		bool HasTextCoords() const { return TextCoords.GetSize() != 0; }
		bool HasIndicies() const { return Indices.GetSize() != 0; }

		const IMeshDeviceProxy* GetMeshProxy() const { return MeshProxy.get(); }
		void UpdateDeviceProxy();

	private:
		Material Mtl;
		Dynarray<Vector3f> Positions;
		Dynarray<Vector3f> Normals;
		Dynarray<Vector2f> TextCoords;
		Dynarray<uint32_t> Indices;

		std::unique_ptr<IMeshDeviceProxy> MeshProxy;
	};
}