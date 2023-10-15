#pragma once
#include <string>
#include <unordered_map>
#include <set>
#include <filesystem>

#include "Refs.h"
#include "CSBridge.h"
#include "IAsset.h"
#include "CSLinked.h"

namespace fs = std::filesystem;

class Game;
class MeshAsset;
class MaterialAsset;
namespace Forms4 {
	class Form;
}

class Assets {
private:
	struct Asset { Ref2 ref; IAsset* iasset; };

public:
	static const std::string ShaderDefault;
	static const std::string ShaderDiffuseColor;
	static const std::string ShaderVertexColor;

	static const std::string ShaderAmbientLight;
	static const std::string ShaderDirectionLight;
	static const std::string ShaderPointLight;
	static const std::string ShaderPointLightMesh;
	static const std::string ShaderSpotLight;

	static const std::string ShaderEditorHihglight;
	static const std::string ShaderEditorOutline;
	static const std::string ShaderBlur;

	static const std::string ShaderWorldGride;

	static const std::string DefaultMaterial;
	static const std::string DefaultDeffuseTexture;

	static const std::string FormBox;
	static const std::string FormBoxLined;
	static const std::string FormSphere;
	static const std::string FormSphereLined;
	static const std::string FormCone;
	static const std::string FormConePath;

	static const std::string Img2x2rgba1111;
	static const std::string Img2x2rgba0001;
	static const std::string Img2x2rgba1001;

private:
	std::unordered_map<int, Asset> m_assets;
	std::set<int> m_dirtyAssets;
	Game* m_game;

public:
	~Assets();

	void Init(Game* game);
	void Start();
	void Destroy();

	bool Contains(const std::string& assetId);
	bool Contains(int assetIdHash);
	
	void Push(const std::string& assetId, IAsset* ptr);

	void Pop(const std::string& assetId);
	void Pop(int assetIdHash);

	IAsset* GetDinamic(const std::string& assetId);
	IAsset* GetDinamic(int assetIdHash);
	const IAsset* GetStatic(const std::string& assetId) { return GetDinamic(assetId); }
	const IAsset* GetStatic(int assetIdHash) { return GetDinamic(assetIdHash); }

	template<typename TAsset>
	TAsset* GetDinamic(const std::string& assetId) { return (TAsset*)GetDinamic(assetId); }
	template<typename TAsset>
	TAsset* GetDinamic(int assetIdHash) { return (TAsset*)GetDinamic(assetIdHash); }
	template<typename TAsset>
	const TAsset* GetStatic(const std::string& assetId) { return (TAsset*)GetDinamic(assetId); }
	template<typename TAsset>
	const TAsset* GetStatic(int assetIdHash) { return (TAsset*)GetDinamic(assetIdHash); }

	bool Load(int assetIdHash, CppRef cppRef = RefCpp(0));

	void ReloadAll();
	void Reload(int assetIdHash);

	void Save(int assetIdHash);

	void MakeDirty(int assetIdHash);
	bool IsDirty(int assetIdHash);

	int GetAssetIDHash(const std::string& str);

private:
	void m_CreateDefaultImages();
	void m_CreateDefaultMaterials();
	void m_CreateDefaultMeshes();

	MeshAsset* m_CreateMeshAssetFromForm(Forms4::Form& form, const std::string& assetId);
};

FUNC(Assets, Save, void)(CppRef gameRef, int assetIdHash);
FUNC(Assets, Reload, void)(CppRef gameRef, int assetIdHash);
FUNC(Assets, Get, CppRef)(CppRef gameRef, int assetIdHash);

FUNC(Assets, MakeDirty, void)(CppRef gameRef, int assetIdHash);
FUNC(Assets, IsDirty, bool)(CppRef gameRef, int assetIdHash);

