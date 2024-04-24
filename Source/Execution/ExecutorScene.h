#pragma once

#include <vector>

#include "../Common.h"

struct Buffer;
struct Texture;

struct Mesh
{
	unsigned NumPrimitives = 0;
	Ptr<Buffer> Positions;
	Ptr<Buffer> Texcoords;
	Ptr<Buffer> Normals;
	Ptr<Buffer> Tangents;
	Ptr<Buffer> Indices;
};

struct SceneObject
{
	Float4x4 ModelTransform;

	Mesh MeshData;

	Ptr<Texture> Albedo;
};

struct Scene
{
	std::vector<SceneObject> SceneObjects;
};