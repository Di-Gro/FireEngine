#pragma once

#include <vector>

#include <directxmath.h>
#include <d3d11.h>
#include <SimpleMath.h>

#include "wrl.h_d3d11_alias.h"
#include "Shader.h"
#include "CsLink.h"
#include "CSBridge.h"
#include "IAsset.h"

#include "ShaderResource.h"
#include "MeshResource.h"

using namespace DirectX::SimpleMath;

class MaterialAsset;

FUNC(MeshAsset, materials_set, void)(CppRef meshRef, size_t* cppRefs, int count);

class MeshAsset : public IAsset {
	FRIEND_FUNC(MeshAsset, materials_set, void)(CppRef meshRef, size_t* cppRefs, int count);

public:
	MeshResource resource;
	std::vector<const MaterialAsset*> f_staticMaterials;

	void Release() override;

public:
	const std::vector<const MaterialAsset*>* GetMaterials() const { return &f_staticMaterials; }
};

FUNC(MeshAsset, ShapeCount, int)(CppRef mesh4Ref);
FUNC(MeshAsset, MaterialMaxIndex, int)(CppRef mesh4Ref);

PUSH_ASSET(MeshAsset);

FUNC(MeshAsset, Init, void)(CppRef gameRef, CppRef meshRef, const char* path);
