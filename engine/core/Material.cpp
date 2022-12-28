#include "Material.h"
#include "Render.h"
#include "Game.h"
#include "Assets.h"
#include "ShaderAsset.h"
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

DEF_PUSH_ASSET(Material);

DEF_PROP_GETSET_F(Material, Vector3, diffuseColor, data.diffuseColor)
DEF_PROP_GETSET_F(Material, float, diffuse, data.diffuse)
DEF_PROP_GETSET_F(Material, float, ambient, data.ambient)
DEF_PROP_GETSET_F(Material, float, specular, data.specular)
DEF_PROP_GETSET_F(Material, float, shininess, data.shininess)
DEF_PROP_GETSET_F(Material, CullMode, cullMode, cullMode)
DEF_PROP_GETSET_F(Material, FillMode, fillMode, fillMode)
DEF_PROP_GETSET_F(Material, size_t, priority, priority)

DEF_FUNC(Material, name_set, void)(CppRef matRef, const char* name) {
	auto material = CppRefs::ThrowPointer<Material>(matRef);
	material->name(name);
}

DEF_FUNC(Material, name_length, size_t)(CppRef matRef) {
	auto material = CppRefs::ThrowPointer<Material>(matRef);
	return material->name().size();
}

DEF_FUNC(Material, name_get, void)(CppRef matRef, char* buf) {
	auto material = CppRefs::ThrowPointer<Material>(matRef);
	auto name = material->name();

	for (int i = 0; i < name.size(); i++)
		buf[i] = name[i];
}

DEF_FUNC(Material, shader_set, void)(CppRef gameRef, CppRef matRef, const char* name) {
	auto game = CppRefs::ThrowPointer<Game>(gameRef);
	auto material = CppRefs::ThrowPointer<Material>(matRef);
	auto hash = game->shaderAsset()->GetShaderHash(name);

	bool hasShader = game->shaderAsset()->HasShader(hash);
	if (!hasShader)
		hasShader = game->shaderAsset()->TryCompileShader(name);

	if (hasShader)
		material->shader = game->shaderAsset()->GetShader(hash);
	else {
		std::string str = "Missing Shader File: ";
		str += name;
		throw std::exception(str.c_str());
	}
}

DEF_FUNC(Material, shader_length, size_t)(CppRef matRef) {
	auto material = CppRefs::ThrowPointer<Material>(matRef);
	if (material->shader == nullptr)
		return 0;
	return material->shader->path.string().size();
}

DEF_FUNC(Material, shader_get, void)(CppRef matRef, char* buf) {
	auto material = CppRefs::ThrowPointer<Material>(matRef);
	auto name = material->shader->path.string();

	for (int i = 0; i < name.size(); i++)
		buf[i] = name[i];
}

DEF_FUNC(Material, isDynamic_get, bool)(CppRef matRef) {
	if (matRef == 0)
		return false;

	auto material = CppRefs::ThrowPointer<Material>(matRef);
	return material->isDynamic;
}

DEF_FUNC(Material, Init, void)(CppRef gameRef, CppRef matRef) {
	auto *game = CppRefs::ThrowPointer<Game>(gameRef);
	auto *material = CppRefs::ThrowPointer<Material>(matRef);

	material->Init(game->render());
}

DEF_FUNC(Material, textures_set, void)(CppRef matRef, size_t* cppRefs, int count) {
	auto* material = CppRefs::ThrowPointer<Material>(matRef);
	
	//TODO: Удалить старые текстуры
	material->textures.clear(); 
	material->resources.clear();

	auto ptr = cppRefs;
	for (int i = 0; i < count; i++, ptr++) {
		auto cppRef = RefCpp(*ptr);
		auto* texture = CppRefs::ThrowPointer<Texture>(cppRef);
		
		material->textures.push_back(texture);
		material->resources.emplace_back(ShaderResource::Create(texture));
	}
}
