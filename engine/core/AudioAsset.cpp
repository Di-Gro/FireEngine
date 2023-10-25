#include "AudioAsset.h"

#include "Game.h"
#include "Scene.h"
#include "Audio.h"
#include "Assets.h"
#include "AssetStore.h"


void AudioAsset::Init(Scene* scene) {
	m_scene = scene;

	m_scene->audio()->AddAsset(this);
}

void AudioAsset::Release() {
	m_scene->audio()->RemoveAsset(this);

	soundEffect = nullptr;
}

void AudioAsset::Load(const std::string& path) {
	soundEffect = m_scene->audio()->LoadSound(path);
}

AudioAsset* AudioAsset::CreateFromFile(Scene* scene, const std::string& path) {
    auto* store = scene->game()->assetStore();
    auto* assets = scene->game()->assets();

    auto& assetId = path;
    auto assetIdHash = assets->GetAssetIDHash(path);
    auto tmpAssetId = scene->ToSceneAssetId(path);

    auto* audioAsset = new AudioAsset();

    assets->Push(tmpAssetId, audioAsset);
    store->AddRuntimeAssetId(tmpAssetId);

    audioAsset->Init(scene);
    audioAsset->Load(path);

    audioAsset->assetId(assetId.c_str());
    audioAsset->assetIdHash(assetIdHash);

    return audioAsset;
}

DEF_PUSH_ASSET(AudioAsset);

DEF_FUNC(AudioAsset, Init, void)(CppRef sceneRef, CppRef assetRef) {
	auto* scene = CppRefs::ThrowPointer<Scene>(sceneRef);
	auto* audioAsset = CppRefs::ThrowPointer<AudioAsset>(assetRef);

	audioAsset->Init(scene);
}

DEF_FUNC(AudioAsset, Load, void)(CppRef assetRef, const char* path) {
	auto* audioAsset = CppRefs::ThrowPointer<AudioAsset>(assetRef);

	audioAsset->Load(path);
}