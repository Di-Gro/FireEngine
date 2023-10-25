#pragma once
#include <string>
#include <xaudio2.h>
#include <Audio.h>

#include "IAsset.h"
#include "CSBridge.h"
#include "Audio.h"

#include "wrl.h_d3d11_alias.h"

class Scene;

class AudioAsset : public IAsset {
	friend class Audio;

public:
	Audio::AssetIter f_sceneIter;

	std::unique_ptr<DirectX::SoundEffect> soundEffect = nullptr;

private:
	Scene* m_scene;

public:
	void Init(Scene* scene);
	void Load(const std::string& path);
	void Release() override;

public:
	static AudioAsset* CreateFromFile(Scene* scene, const std::string& path);
};

PUSH_ASSET(AudioAsset);
FUNC(AudioAsset, Init, void)(CppRef sceneRef, CppRef assetRef);
FUNC(AudioAsset, Load, void)(CppRef assetRef, const char* path);