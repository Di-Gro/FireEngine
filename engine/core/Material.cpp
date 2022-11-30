#include "Material.h"

DEF_FUNC(Material, diffuseColor_get, Vector3)(CppRef matRef) {
	return CppRefs::ThrowPointer<Material>(matRef)->data.diffuseColor;
}

DEF_FUNC(Material, diffuse_get, float)(CppRef matRef) {
	return CppRefs::ThrowPointer<Material>(matRef)->data.diffuse;
}

DEF_FUNC(Material, ambient_get, float)(CppRef matRef) {
	return CppRefs::ThrowPointer<Material>(matRef)->data.ambient;
}

DEF_FUNC(Material, specular_get, float)(CppRef matRef) {
	return CppRefs::ThrowPointer<Material>(matRef)->data.specular;
}

DEF_FUNC(Material, shininess_get, float)(CppRef matRef) {
	return CppRefs::ThrowPointer<Material>(matRef)->data.shininess;
}


DEF_FUNC(Material, diffuseColor_set, void)(CppRef matRef, Vector3 value) {
	CppRefs::ThrowPointer<Material>(matRef)->data.diffuseColor = value;
}

DEF_FUNC(Material, diffuse_set, void)(CppRef matRef, float value) {
	CppRefs::ThrowPointer<Material>(matRef)->data.diffuse = value;
}

DEF_FUNC(Material, ambient_set, void)(CppRef matRef, float value) {
	CppRefs::ThrowPointer<Material>(matRef)->data.ambient = value;
}

DEF_FUNC(Material, specular_set, void)(CppRef matRef, float value) {
	CppRefs::ThrowPointer<Material>(matRef)->data.specular = value;
}

DEF_FUNC(Material, shininess_set, void)(CppRef matRef, float value) {
	CppRefs::ThrowPointer<Material>(matRef)->data.shininess = value;
}

//DEF_FUNC(Material, name_get, const char*)(CppRef matRef) {
//	return CppRefs::ThrowPointer<Material>(matRef)->name_cstr();
//}

DEF_FUNC(Material, name_get, int)(CppRef matRef, char* buf, int bufLehgth) {
	auto material = CppRefs::ThrowPointer<Material>(matRef);

	auto dynamicPrefix = material->isDynamic ? "[D] " : "";
	auto name = dynamicPrefix + material->name();

	int writeIndex = 0;
	for (; writeIndex < bufLehgth - 1 && writeIndex < name.size(); writeIndex++)
		buf[writeIndex] = name[writeIndex];
	buf[writeIndex] = '\0';

	return writeIndex;
}
//
//char* _cdecl Material_name_get(CppRef matRef) {
//	return (char*)CppRefs::ThrowPointer<Material>(matRef)->name_cstr();
//}