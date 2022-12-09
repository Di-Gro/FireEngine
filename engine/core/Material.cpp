#include "Material.h"
#include "Render.h"


void Material::Init(Render* render) {

	m_render = render;

	D3D11_BUFFER_DESC materialCBufferDesc = {};
	materialCBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialCBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialCBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	materialCBufferDesc.MiscFlags = 0;
	materialCBufferDesc.StructureByteStride = 0;
	materialCBufferDesc.ByteWidth = sizeof(Material::Data);

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;

	render->device()->CreateBuffer(&materialCBufferDesc, nullptr, materialConstBuffer.GetAddressOf());

	render->device()->CreateRasterizerState(&rastDesc, rastState.GetAddressOf());

	UpdateDepthStencilState();
}

void Material::UpdateDepthStencilState() {
	if (depthStencilState.Get() != nullptr)
		depthStencilState.ReleaseAndGetAddressOf();

	m_render->device()->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());
}


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