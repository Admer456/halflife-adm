
#include "cl_dll.h"
#include "cl_util.h"
#include "physics/Physics.hpp"
#include "physics/DebugRenderer.hpp"
#include "triangleapi.h"

using namespace Physics;

BatchImpl::BatchImpl(const JPH::DebugRenderer::Triangle* triangles, int triangleCount)
{
	vertexPositions.reserve(triangleCount * 3);
	vertexColours.reserve(triangleCount * 3);
	vertexIndices.reserve(triangleCount * 3);

	int vertexCount = 0;

	for ( int i = 0; i < triangleCount; i++ )
	{
		for (const auto& v : triangles[i].mV)
		{
			vertexPositions.emplace_back(v.mPosition.x, v.mPosition.y, v.mPosition.z, 1.0f);
			vertexColours.emplace_back(RGBA{v.mColor.r, v.mColor.g, v.mColor.b, v.mColor.a});

			vertexIndices.push_back(vertexCount);
			vertexCount++;
		}
	}
}

BatchImpl::BatchImpl(const JPH::DebugRenderer::Vertex* vertices, int vertexCount, const uint32* indices, int indexCount)
{
	vertexPositions.reserve(vertexCount);
	vertexColours.reserve(vertexCount);
	vertexIndices.reserve(indexCount);

	for ( int i = 0; i < vertexCount; i++ )
	{
		const auto& v = vertices[i];
		vertexPositions.emplace_back(v.mPosition.x, v.mPosition.y, v.mPosition.z, 1.0f);
		vertexColours.emplace_back(RGBA{v.mColor.r, v.mColor.g, v.mColor.b, v.mColor.a});
	}

	vertexIndices.insert(vertexIndices.begin(), indices, indices + indexCount);
}

DebugRendererImpl::DebugRendererImpl(triangleapi_t*& engineTriApi)
	: TriApi(engineTriApi)
{
	DebugRenderer::Initialize();
}

void DebugRendererImpl::DrawLine(
	const JPH::Float3& inFrom,
	const JPH::Float3& inTo,
	JPH::ColorArg inColor)
{
	TriApi->CullFace(TRI_NONE);
	TriApi->RenderMode(kRenderTransColor);
	TriApi->Color4ub(inColor.r, inColor.g, inColor.b, 100);
	TriApi->Begin(TRI_LINES);

	TriApi->Vertex3fv(mtou(Vector(&inFrom.x)));
	TriApi->Vertex3fv(mtou(Vector(&inTo.x)));

	TriApi->End();
}

void DebugRendererImpl::DrawTriangle(
	JPH::Vec3Arg inV1,
	JPH::Vec3Arg inV2,
	JPH::Vec3Arg inV3,
	JPH::ColorArg inColor)
{
	TriApi->CullFace(TRI_NONE);
	TriApi->RenderMode(kRenderTransColor);
	TriApi->Color4ub(inColor.r, inColor.g, inColor.b, 100);
	TriApi->Begin(TRI_TRIANGLES);

	TriApi->Vertex3fv(mtou(Vector(&inV1.mF32[0])));
	TriApi->Vertex3fv(mtou(Vector(&inV2.mF32[0])));
	TriApi->Vertex3fv(mtou(Vector(&inV3.mF32[0])));
	
	TriApi->End();
}

JPH::DebugRenderer::Batch DebugRendererImpl::CreateTriangleBatch(
	const Triangle* inTriangles,
	int inTriangleCount)
{
	return new BatchImpl(inTriangles, inTriangleCount);
}

JPH::DebugRenderer::Batch DebugRendererImpl::CreateTriangleBatch(
	const Vertex* inVertices,
	int inVertexCount,
	const uint32* inIndices,
	int inIndexCount)
{
	return new BatchImpl(inVertices, inVertexCount, inIndices, inIndexCount);
}

void DebugRendererImpl::DrawGeometry(
	JPH::Mat44Arg inModelMatrix,
	const JPH::AABox& inWorldSpaceBounds,
	float inLODScaleSq,
	JPH::ColorArg inModelColor,
	const GeometryRef& inGeometry,
	ECullMode inCullMode,
	ECastShadow inCastShadow,
	EDrawMode inDrawMode)
{
	const bool wireframe = inDrawMode == EDrawMode::Wireframe;
	const BatchImpl* batch = static_cast<BatchImpl*>(inGeometry->mLODs[0].mTriangleBatch.GetPtr());

	TriApi->CullFace(TRI_NONE);
	TriApi->RenderMode(kRenderTransTexture);
	TriApi->Color4ub(inModelColor.r, inModelColor.g, inModelColor.b, 100);
	TriApi->Begin(!wireframe ? TRI_TRIANGLES : TRI_LINES);

	for (size_t i = 0; i < batch->vertexIndices.size(); i += 3)
	{
		const JPH::Vec4 transformed[3]
		{
			mtou(inModelMatrix * batch->vertexPositions[batch->vertexIndices[i+0]]),
			mtou(inModelMatrix * batch->vertexPositions[batch->vertexIndices[i+1]]),
			mtou(inModelMatrix * batch->vertexPositions[batch->vertexIndices[i+2]])
		};

		// Wireframe is more expensive to render, but usually these are low-poly objects, so it should be fine
		if (wireframe)
		{
			TriApi->Vertex3fv(&transformed[0].mF32[0]);
			TriApi->Vertex3fv(&transformed[1].mF32[0]);
			TriApi->Vertex3fv(&transformed[1].mF32[0]);
			TriApi->Vertex3fv(&transformed[2].mF32[0]);
			TriApi->Vertex3fv(&transformed[2].mF32[0]);
			TriApi->Vertex3fv(&transformed[0].mF32[0]);
		}
		else
		{
			TriApi->Vertex3fv(&transformed[0].mF32[0]);
			TriApi->Vertex3fv(&transformed[1].mF32[0]);
			TriApi->Vertex3fv(&transformed[2].mF32[0]);
		}
	}

	TriApi->End();
}
