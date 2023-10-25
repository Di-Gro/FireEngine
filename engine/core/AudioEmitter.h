#pragma once

#include <Audio.h>

#include "Actor.h"
#include "Audio.h"
#include "Sound.h"


FUNC(AudioEmitter, SetSound, void)(CppRef compRef, CppRef soundRef);
//FUNC(AudioEmitter, SetPreInitSound, void)(CppRef compRef, CppRef soundRef);

class AudioEmitter : public Component, public ISound {
	friend class Audio;

	COMPONENT(AudioEmitter);
	FRIEND_FUNC(AudioEmitter, SetSound, void)(CppRef compRef, CppRef soundRef);
	//FRIEND_FUNC(AudioEmitter, SetPreInitSound, void)(CppRef compRef, CppRef soundRef);
public:
	float CurveDistanceScaler = 3;
	float DopplerScaler = 0;
	float InnerRadius = 50;

private:
	Audio::EmitterIter f_sceneIter;

	DirectX::AudioEmitter m_emitter;

	Sound* m_sound = nullptr;
	Sound* m_preInitSound = nullptr;
	bool m_isAdded = false;

public:
	void OnPreInit();

	void OnInit() override;
	void OnDestroy() override;
	void OnInitDisabled() override { OnInit(); }
	void OnDestroyDisabled() override { OnDestroy(); }

	void OnActivate() override;
	void OnDeactivate() override;

	Sound* sound() { return m_sound; }
	void sound(Sound* value);

	DirectX::SoundEffectInstance* soundInstance() override;

	void Apply3D(const DirectX::AudioListener& listener, float dt);

private:
	bool m_TrySetSound(Sound* value);

};
DEC_COMPONENT(AudioEmitter);

//FUNC(AudioEmitter, OnPreInit, void)(CppRef compRef);

PROP_GETSET(AudioEmitter, bool, loop);

PROP_GET(AudioEmitter, bool, IsStopped);
PROP_GET(AudioEmitter, bool, IsPlaying);
PROP_GET(AudioEmitter, bool, IsPaused);

FUNC(AudioEmitter, Play, void)(CppRef iSoundRef);
FUNC(AudioEmitter, Stop, void)(CppRef iSoundRef);
FUNC(AudioEmitter, Pause, void)(CppRef iSoundRef);
FUNC(AudioEmitter, Resume, void)(CppRef iSoundRef);
