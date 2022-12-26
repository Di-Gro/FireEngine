#include "Material.h"
#include "Render.h"
#include "Game.h"
#include "Assets.h"
#include "Material.h"


void Material::Release() {
	if (materialConstBuffer.Get() != nullptr)
		materialConstBuffer.ReleaseAndGetAddressOf();

	if (depthStencilState.Get() != nullptr)
		depthStencilState.ReleaseAndGetAddressOf();
}

void Material::Init(Render* render) {

	m_render = render;

	D3D11_BUFFER_DESC materialCBufferDesc = {};
	materialCBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialCBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialCBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	materialCBufferDesc.MiscFlags = 0;
	materialCBufferDesc.StructureByteStride = 0;
	materialCBufferDesc.ByteWidth = sizeof(Material::Data);

	depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;

	render->device()->CreateBuffer(&materialCBufferDesc, nullptr, materialConstBuffer.GetAddressOf());

	UpdateDepthStencilState();
}

void Material::UpdateDepthStencilState() {
	if (depthStencilState.Get() != nullptr)
		depthStencilState.ReleaseAndGetAddressOf();

	m_render->device()->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());
}

DEF_PROP_GETSET_F(Material, int, pathHash, pathHash);

DEF_PROP_GETSET_F(Material, Vector3, diffuseColor, data.diffuseColor)
DEF_PROP_GETSET_F(Material, float, diffuse, data.diffuse)
DEF_PROP_GETSET_F(Material, float, ambient, data.ambient)
DEF_PROP_GETSET_F(Material, float, specular, data.specular)
DEF_PROP_GETSET_F(Material, float, shininess, data.shininess)
DEF_PROP_GETSET_F(Material, CullMode, cullMode, cullMode)
DEF_PROP_GETSET_F(Material, FillMode, fillMode, fillMode)
DEF_PROP_GETSET_F(Material, size_t, priority, priority)

DEF_FUNC(Material, name_get, int)(CppRef matRef, char* buf, int bufLehgth) {
	if (matRef == 0)
		return 0;

	auto material = CppRefs::ThrowPointer<Material>(matRef);

	auto dynamicPrefix = material->isDynamic ? "[D] " : "";
	auto name = dynamicPrefix + material->name();

	int writeIndex = 0;
	for (; writeIndex < bufLehgth - 1 && writeIndex < name.size(); writeIndex++)
		buf[writeIndex] = name[writeIndex];
	buf[writeIndex] = '\0';

	return writeIndex;
}

DEF_FUNC(Material, isDynamic_get, bool)(CppRef matRef) {
	if (matRef == 0)
		return false;

	auto material = CppRefs::ThrowPointer<Material>(matRef);
	return material->isDynamic;
}

DEF_FUNC(Material, Create, CppRef)(CppRef gameRef, size_t assetHash) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);

	auto* material = (Material*)game->assets()->Get(assetHash);
	if (material == nullptr) {
		material = new Material();
		material->Init(game->render());
		game->assets()->Push(assetHash, material);
	}
	return CppRefs::GetRef(material);
}
