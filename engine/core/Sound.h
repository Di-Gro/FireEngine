#pragma once

#include <Audio.h>

#include "Audio.h"
#include "ISound.h"
#include "CsLink.h"

class AudioAsset;
class Scene;

FUNC(Sound, SetAsset, void)(CppRef soundRef, CppRef assetRef);

class Sound : public CsLink, public ISound {
	friend class Audio;

	FRIEND_FUNC(Sound, SetAsset, void)(CppRef soundRef, CppRef assetRef);

private:
	Audio::SoundIter f_sceneIter;

	Scene* m_scene;
	const AudioAsset* m_asset = nullptr;
	std::unique_ptr<DirectX::SoundEffectInstance> m_soundInstance = nullptr;

	bool m_use3d = false;

public:
	Sound(Scene* scene, CsRef csRef = RefCs(0));
	~Sound();

	const AudioAsset* asset() { return m_asset; }
	void asset(const AudioAsset* value);

	DirectX::SoundEffectInstance* soundInstance() override { return m_soundInstance.get(); }

	void Use3D(bool value);

private:
	bool m_TrySetAsset(const AudioAsset* value);
	std::unique_ptr<DirectX::SoundEffectInstance> m_CreateSoundInstance();
};

PROP_GETSET(Sound, bool, loop);

PROP_GET(Sound, bool, IsStopped);
PROP_GET(Sound, bool, IsPlaying);
PROP_GET(Sound, bool, IsPaused);

FUNC(Sound, Play, void)(CppRef iSoundRef);
FUNC(Sound, Stop, void)(CppRef iSoundRef);
FUNC(Sound, Pause, void)(CppRef iSoundRef);
FUNC(Sound, Resume, void)(CppRef iSoundRef);