#pragma once
#include <string>
#include <unordered_map>

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
	Game* m_game;

	mono::mono_method_invoker<void(int)> m_method_Load;
	mono::mono_method_invoker<void(int)> m_method_Reload;
	mono::mono_method_invoker<int(size_t, size_t)> m_method_GetStringHash;
	mono::mono_method_invoker<unsigned int(void)> m_method_CreateAssetId;

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

	void ReloadAll();
	void Reload(int assetIdHash);

	int GetCsHash(const std::string& str);
	std::string CreateAssetId();

		
};

FUNC(Assets, Reload, void)(CppRef gameRef, int assetIdHash);
FUNC(Assets, Get, CppRef)(CppRef gameRef, int assetIdHash);
