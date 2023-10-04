#pragma once
#include <string>
#include <unordered_map>
#include <set>

#include "FileSystem.h"
#include "Refs.h"
#include "CSBridge.h"
#include "IAsset.h"
#include "CSLinked.h"

class Game;

class Assets {
private:
	struct Asset { Ref2 ref; IAsset* ptr; };

public:
	static std::string ShaderDefault;
	static std::string ShaderDiffuseColor;
	static std::string ShaderVertexColor;

	static std::string ShaderAmbientLight;
	static std::string ShaderDirectionLight;
	static std::string ShaderPointLight;
	static std::string ShaderPointLightMesh;
	static std::string ShaderSpotLight;

	static std::string ShaderEditorHihglight;
	static std::string ShaderEditorOutline;
	static std::string ShaderBlur;

	static std::string ShaderWorldGride;

private:
	std::unordered_map<int, Asset> m_assets;
	std::set<int> m_dirtyAssets;
	Game* m_game;

	//mono::mono_method_invoker<void(int)> m_method_Load;
	//mono::mono_method_invoker<void(int)> m_method_Reload;
	//mono::mono_method_invoker<int(size_t, size_t)> m_method_GetStringHash;
	//mono::mono_method_invoker<unsigned int(void)> m_method_CreateAssetId;
	mono::mono_method_invoker<unsigned int(void)> m_method_CreateTmpAssetId;
	mono::mono_method_invoker<void(unsigned int)> m_method_AddTmpAssetIdHash;

public:
	~Assets();

	void Init(Game* game);

	bool Contains(const std::string& assetId);
	bool Contains(int assetIdHash);
	
	void Push(const std::string& assetId, IAsset* ptr);
	void Push(int assetIdHash, IAsset* ptr);

	IAsset* Pop(const std::string& assetId);
	IAsset* Pop(int assetIdHash);

	IAsset* Get(const std::string& assetId);
	IAsset* Get(int assetIdHash);

	bool Load(int assetIdHash, CppRef cppRef = RefCpp(0));

	void ReloadAll();
	void Reload(int assetIdHash);

	void Save(int assetIdHash);

	void MakeDirty(int assetIdHash);
	bool IsDirty(int assetIdHash);

	int GetCsAssetIDHash(const std::string& str);
	//std::string CreateAssetId();
	std::string CreateTmpAssetId();

		
};

FUNC(Assets, Reload, void)(CppRef gameRef, int assetIdHash);
FUNC(Assets, Get, CppRef)(CppRef gameRef, int assetIdHash);

FUNC(Assets, MakeDirty, void)(CppRef gameRef, int assetIdHash);
FUNC(Assets, IsDirty, bool)(CppRef gameRef, int assetIdHash);
