#include "MeshAsset.h"

#include "Game.h"
#include "Assets.h"
#include "MaterialAsset.h"

void MeshAsset::Release() {
	resource.Release();
}

DEF_FUNC(MeshAsset, ShapeCount, int)(CppRef mesh4Ref) {
	if (mesh4Ref == 0)
		return 0;

	return CppRefs::ThrowPointer<MeshAsset>(mesh4Ref)->resource.shapeCount();
}

DEF_FUNC(MeshAsset, MaxMaterialIndex, int)(CppRef mesh4Ref) {
	if (mesh4Ref == 0)
		return 0;

	return CppRefs::ThrowPointer<MeshAsset>(mesh4Ref)->resource.maxMaterialIndex();
}

DEF_PUSH_ASSET(MeshAsset);

DEF_FUNC(MeshAsset, Init, void)(CppRef gameRef, CppRef meshRef, const char* path) {
	auto* game = CppRefs::ThrowPointer<Game>(gameRef);
	auto* mesh = CppRefs::ThrowPointer<MeshAsset>(meshRef);

	auto version = mesh->resource.version++;

	mesh->resource = MeshResource::CreateFromObj(game->render(), path);
	mesh->resource.version = version;
}

DEF_FUNC(MeshAsset, materials_set, void)(CppRef meshRef, size_t* cppRefs, int count) {
	auto* mesh = CppRefs::ThrowPointer<MeshAsset>(meshRef);

	mesh->f_staticMaterials.clear();

	auto ptr = cppRefs;
	for (int i = 0; i < count; i++, ptr++) {
		auto* material = MaterialAsset::Default;

		if (*ptr != 0) {
			auto cppRef = RefCpp(*ptr);
			material = CppRefs::ThrowPointer<MaterialAsset>(cppRef);
		}
		mesh->f_staticMaterials.push_back(material);
	}
}

DEF_FUNC(MeshAsset, NextVersion, void)(CppRef meshRef) {
	auto* mesh = CppRefs::ThrowPointer<MeshAsset>(meshRef);
	mesh->resource.version++;
}