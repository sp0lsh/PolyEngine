#include <CorePCH.hpp>

#include <Math/AABox.hpp>

using namespace Poly;

//------------------------------------------------------------------------------
AABox::AABox(const Vector& min, const Vector& size)
	: Min(min), Size(size)
{
	ASSERTE(Size.X >= 0 && Size.Y >= 0 && Size.Z >= 0, "Invalid aabox size!");
}

//------------------------------------------------------------------------------
AABox AABox::GetIntersectionVolume(const AABox& rhs) const
{
	const float r1MinX = std::min(Min.X, Min.X + Size.X);
	const float r1MaxX = std::max(Min.X, Min.X + Size.X);
	const float r1MinY = std::min(Min.Y, Min.Y + Size.Y);
	const float r1MaxY = std::max(Min.Y, Min.Y + Size.Y);
	const float r1MinZ = std::min(Min.Z, Min.Z + Size.Z);
	const float r1MaxZ = std::max(Min.Z, Min.Z + Size.Z);

	const float r2MinX = std::min(rhs.Min.X, rhs.Min.X + rhs.Size.X);
	const float r2MaxX = std::max(rhs.Min.X, rhs.Min.X + rhs.Size.X);
	const float r2MinY = std::min(rhs.Min.Y, rhs.Min.Y + rhs.Size.Y);
	const float r2MaxY = std::max(rhs.Min.Y, rhs.Min.Y + rhs.Size.Y);
	const float r2MinZ = std::min(rhs.Min.Z, rhs.Min.Z + rhs.Size.Z);
	const float r2MaxZ = std::max(rhs.Min.Z, rhs.Min.Z + rhs.Size.Z);

	const float interLeft = std::max(r1MinX, r2MinX);
	const float interTop = std::max(r1MinY, r2MinY);
	const float interUp = std::max(r1MinZ, r2MinZ);
	const float interRight = std::min(r1MaxX, r2MaxX);
	const float interBottom = std::min(r1MaxY, r2MaxY);
	const float interDown = std::min(r1MaxZ, r2MaxZ);

	if ((interLeft < interRight) && (interTop < interBottom) && (interUp < interDown))
		return AABox(Vector(interLeft, interTop, interUp), Vector(interRight - interLeft, interBottom - interTop, interDown - interUp));
	else
		return AABox(Vector::ZERO, Vector::ZERO);
}

std::array<Vector, 8> Poly::AABox::GetVertices() const
{
    SILENCE_CLANG_WARNING(-Wmissing-braces, "Everything is ok here.");
	return std::array<Vector, 8>{
	    Min,
		Min + Vector(Size.X, 0, 0),
		Min + Vector(Size.X, Size.Y, 0),
		Min + Vector(Size.X, Size.Y, Size.Z),
		Min + Vector(0, Size.Y, Size.Z),
		Min + Vector(0, 0, Size.Z),
		Min + Vector(0, Size.Y, 0),
		Min + Vector(Size.X, 0, Size.Z)
	};
    UNSILENCE_CLANG_WARNING();
}

AABox Poly::AABox::GetTransformed(const Matrix& transform) const
{
	Vector min = transform * Min;
	Vector max = min;

	// Gather other 7 points
    SILENCE_CLANG_WARNING(-Wmissing-braces, "Everything is ok here.");
	std::array<Vector, 7> points =
	{
		Min + Vector(Size.X, 0, 0),
		Min + Vector(Size.X, Size.Y, 0),
		Min + Vector(Size.X, Size.Y, Size.Z),
		Min + Vector(0, Size.Y, Size.Z),
		Min + Vector(0, 0, Size.Z),
		Min + Vector(0, Size.Y, 0),
		Min + Vector(Size.X, 0, Size.Z)
	};
    UNSILENCE_CLANG_WARNING();

	// Iterate other 7 points
	for (Vector v : points)
	{
		v = transform * v;
		min = Vector::Min(min, v);
		max = Vector::Max(max, v);
	}

	return AABox(min, max - min);
}

Poly::AABox& Poly::AABox::Expand(const AABox& rhs)
{
	Vector min = Vector::Min(GetMin(), rhs.GetMin());
	Vector max = Vector::Max(GetMax(), rhs.GetMax());
	Min = min;
	Size = max - min;
	return *this;
}

Poly::AABox& Poly::AABox::Expand(const Vector& rhs)
{
	Vector min = Vector::Min(GetMin(), rhs);
	Vector max = Vector::Max(GetMax(), rhs);
	Min = min;
	Size = max - min;
	return *this;
}

//------------------------------------------------------------------------------
namespace Poly {
	std::ostream & operator<<(std::ostream& stream, const AABox& rect)
	{
		return stream << "AABox[Pos: " << rect.Min << " Size: " << rect.Size << " ]";
	}
}
