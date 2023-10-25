#include "Sound.h"

#include "AudioAsset.h"
#include "Game.h"
#include "Scene.h"

Sound::Sound(Scene* scene, CsRef csRef) {
	m_scene = scene;
	m_scene->audio()->AddSound(this, csRef);
}

Sound::~Sound() {
	m_scene->audio()->RemoveSound(this);
}

void Sound::asset(const AudioAsset* value) {
	if (m_TrySetAsset(value)) {
		auto assetRef = CppRefs::GetRef(m_asset);
		m_scene->game()->callbacks().soundSetAsset(csRef(), assetRef);
	}
}

bool Sound::m_TrySetAsset(const AudioAsset* value) {
	if (m_asset == value)
		return false;

	m_asset = value;

	if (m_asset == nullptr)
		m_soundInstance = nullptr;
	else
		m_soundInstance = m_CreateSoundInstance();

	return true;
}

void Sound::Use3D(bool value) {
	if (m_use3d == value)
		return;

	m_use3d = value;

	if (m_soundInstance == nullptr)
		return;

	m_soundInstance = m_CreateSoundInstance();
}

std::unique_ptr<DirectX::SoundEffectInstance> Sound::m_CreateSoundInstance() {
	if (m_use3d)
		return m_asset->soundEffect->CreateInstance(DirectX::SoundEffectInstance_Use3D);
	
	return m_asset->soundEffect->CreateInstance(DirectX::SoundEffectInstance_Default);
}

DEF_FUNC(Sound, SetAsset, void)(CppRef soundRef, CppRef assetRef) {
	auto* sound = CppRefs::ThrowPointer<Sound>(soundRef);
	auto* asset = CppRefs::GetPointer<AudioAsset>(assetRef);

	sound->m_TrySetAsset(asset);
}

DEF_PROP_GETSET(Sound, bool, loop);

DEF_PROP_GET(Sound, bool, IsStopped);
DEF_PROP_GET(Sound, bool, IsPlaying);
DEF_PROP_GET(Sound, bool, IsPaused);

DEF_FUNC(Sound, Play, void)(CppRef iSoundRef) {
	auto iSound = CppRefs::ThrowPointer<Sound>(iSoundRef);

	iSound->Play();
}

DEF_FUNC(Sound, Stop, void)(CppRef iSoundRef) {
	CppRefs::ThrowPointer<Sound>(iSoundRef)->Stop();
}

DEF_FUNC(Sound, Pause, void)(CppRef iSoundRef) {
	CppRefs::ThrowPointer<Sound>(iSoundRef)->Pause();
}

DEF_FUNC(Sound, Resume, void)(CppRef iSoundRef) {
	CppRefs::ThrowPointer<Sound>(iSoundRef)->Resume();
}