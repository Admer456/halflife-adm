
#pragma once

typedef struct triangleapi_s triangleapi_t;

namespace Physics
{
// A simple mesh for debug rendering
class BatchImpl final : public JPH::RefTargetVirtual, JPH::RefTarget<BatchImpl>
{
public:
	BatchImpl(const JPH::DebugRenderer::Triangle* triangles, int triangleCount);
	BatchImpl(const JPH::DebugRenderer::Vertex* vertices, int vertexCount, const uint32* indices, int indexCount);

	void AddRef() override
	{
		JPH::RefTarget<BatchImpl>::AddRef();
	}

	void Release() override
	{
		if ( --mRefCount == 0 )
		{
			delete this;
		}
	}

	// It's Vec4 so we can multiply it by the model matrix.
	// Usage of matrices in TriAPI is unclear, so we do it manually
	std::vector<JPH::Vec4> vertexPositions;
	std::vector<::RGBA> vertexColours;
	std::vector<uint32> vertexIndices;
};

// A renderer that uses TriAPI to draw debug overlays for JoltPhysics
class DebugRendererImpl final : public JPH::DebugRenderer
{
public:
	DebugRendererImpl(triangleapi_t*& engineTriApi);

	void DrawLine(
		const JPH::Float3& inFrom,
		const JPH::Float3& inTo,
		JPH::ColorArg      inColor) override;

	void DrawTriangle(
		JPH::Vec3Arg  inV1,
		JPH::Vec3Arg  inV2,
		JPH::Vec3Arg  inV3,
		JPH::ColorArg inColor) override;

	Batch CreateTriangleBatch(
		const Triangle* inTriangles,
		int             inTriangleCount) override;

	Batch CreateTriangleBatch(
		const Vertex* inVertices,
		int           inVertexCount,
		const uint32* inIndices,
		int           inIndexCount) override;

	void DrawGeometry(
		JPH::Mat44Arg      inModelMatrix,
		const JPH::AABox&  inWorldSpaceBounds,
		float              inLODScaleSq,
		JPH::ColorArg      inModelColor,
		const GeometryRef& inGeometry,
		ECullMode          inCullMode = ECullMode::CullBackFace,
		ECastShadow        inCastShadow = ECastShadow::On,
		EDrawMode          inDrawMode = EDrawMode::Solid) override;

	void DrawText3D(
		JPH::Vec3Arg            inPosition,
		const std::string_view& inString,
		JPH::ColorArg           inColor = JPH::Color::sWhite,
		float                   inHeight = 0.5f) override
	{
		// No.
		return;
	}

private:
	triangleapi_t*& TriApi;
};
};
