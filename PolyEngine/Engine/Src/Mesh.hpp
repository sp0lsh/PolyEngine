#pragma once

#include <BaseObject.hpp>
#include <Dynarray.hpp>
#include <Vector.hpp>
#include <Vector2f.hpp>
#include <Color.hpp>
#include <Matrix.hpp>

typedef unsigned int GLuint;

namespace Poly
{
	class TextureResource;
	class IMeshDeviceProxy;

	// class ENGINE_DLLEXPORT Matrix4x4 : public BaseObjectLiteralType<> {
	// public:
	// 	// constexpr Vector3f() : X(0.f), Y(0.f), Z(0.f) {}
	// 	// constexpr Vector3f(float x, float y, float z) : X(x), Y(y), Z(z) {}
	// 	Matrix4x4(const float values[16]) {
	// 		memmove(Data, values, sizeof(float) * 16);
	// 	};
	// 	// float Data[16];
	// 	// alignas(16) std::array<float, 16> Data;
	// 	// std::array<float, 16> Data;
	// 	float Data[16];
	// };


	class ENGINE_DLLEXPORT Mesh : public BaseObject<>
	{
	public:
		virtual ~Mesh();

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
		size_t GetInstancesCount() const { return Instances.GetSize() / 16; }

		const Dynarray<Vector3f>& GetPositions() const { return Positions; }
		const Dynarray<Vector3f>& GetNormals() const { return Normals; }
		const Dynarray<Vector2f>& GetTextCoords() const { return TextCoords; }
		const Dynarray<uint32_t>& GetIndicies() const { return Indices; }
		const Dynarray<float>& GetInstances() const { return Instances; }

		void SetPositions(const Dynarray<Vector3f>& values) { Positions = values; }
		void SetNormals(const Dynarray<Vector3f>& values) { Normals = values; }
		void SetTextCoords(const Dynarray<Vector2f>& values) { TextCoords = values; }
		void SetIndicies(const Dynarray<uint32_t>& values) { Indices = values; }
		void SetInstances(const Dynarray<float>& values) { Instances = values; }

		bool HasVertices() const { return Positions.GetSize() != 0; }
		bool HasNormals() const { return Normals.GetSize() != 0; }
		bool HasTextCoords() const { return TextCoords.GetSize() != 0; }
		bool HasIndicies() const { return Indices.GetSize() != 0; }
		bool HasInstances() const { return Instances.GetSize() != 0; }

		const IMeshDeviceProxy* GetMeshProxy() const { return MeshProxy.get(); }
		void UpdateDeviceProxy();

	private:
		Material Mtl;
		Dynarray<Vector3f> Positions;
		Dynarray<Vector3f> Normals;
		Dynarray<Vector2f> TextCoords;
		Dynarray<uint32_t> Indices;
		Dynarray<float> Instances;

		std::unique_ptr<IMeshDeviceProxy> MeshProxy;
	};
}