#include "AudioEmitter.h"

#include "Game.h"
#include "Scene.h"


void AudioEmitter::OnPreInit() {
	if (m_preInitSound != nullptr) {
		sound(m_preInitSound);
		m_preInitSound = nullptr;
	}
}

void AudioEmitter::OnInit() {
	OnPreInit();

	if (!m_isAdded) {
		m_isAdded = true;

		scene()->audio()->AddEmitter(this);
	}
} 

void AudioEmitter::OnDestroy() {
	if (m_isAdded) {
		m_isAdded = false;

		Stop();
		scene()->audio()->RemoveEmitter(this);
	}
}

void AudioEmitter::OnActivate() {
	if (!m_isAdded) {
		m_isAdded = true;

		if (IsPaused())
			Resume();

		scene()->audio()->AddEmitter(this);
	}
}

void AudioEmitter::OnDeactivate() {
	if (m_isAdded) {
		m_isAdded = false;

		Pause();
		scene()->audio()->RemoveEmitter(this);
	}
}

void AudioEmitter::sound(Sound* value) {
	if (m_TrySetSound(value)) {
		auto soundRef = m_sound == nullptr ? RefCs(0) : m_sound->csRef();
		game()->callbacks().emitterSetSound(csRef(), soundRef);
	}
}

bool AudioEmitter::m_TrySetSound(Sound* value) {
	if (m_sound == value)
		return false;

	bool isPlaying = false;
	bool isPaused = false;

	if (m_sound != nullptr) {
		isPlaying = m_sound->IsPlaying();
		isPaused = m_sound->IsPaused();

		m_sound->Use3D(false);
	}
	m_sound = value;
	if (m_sound != nullptr) {
		m_sound->Use3D(true);

		if (isPlaying || isPaused)
			Play();

		if (isPaused)
			Pause();
	}
	return true;
}

DirectX::SoundEffectInstance* AudioEmitter::soundInstance() {
	if (m_sound == nullptr || CppRefs::GetRef(m_sound).value == 0)
		return nullptr;

	return m_sound->soundInstance();
}

void AudioEmitter::Apply3D(const DirectX::AudioListener& listener, float dt) {
	auto* instance = soundInstance();
	if (instance == nullptr)
		return;

	m_emitter.CurveDistanceScaler = CurveDistanceScaler;
	m_emitter.DopplerScaler = DopplerScaler;
	m_emitter.InnerRadius = InnerRadius;
	m_emitter.ChannelCount = 2;
	m_emitter.Update(worldPosition(), up(), dt);

	instance->Apply3D(listener, m_emitter);
}

DEF_COMPONENT(AudioEmitter, Engine.AudioEmitter, 3, RunMode::EditPlay) {
	OFFSET(0, AudioEmitter, CurveDistanceScaler);
	OFFSET(1, AudioEmitter, DopplerScaler);
	OFFSET(2, AudioEmitter, InnerRadius);
}

DEF_FUNC(AudioEmitter, SetSound, void)(CppRef compRef, CppRef soundRef) {
	auto* comp = CppRefs::ThrowPointer<AudioEmitter>(compRef);
	auto* sound = CppRefs::GetPointer<Sound>(soundRef);

	comp->m_TrySetSound(sound);
}

DEF_PROP_GETSET(AudioEmitter, bool, loop);

DEF_PROP_GET(AudioEmitter, bool, IsStopped);
DEF_PROP_GET(AudioEmitter, bool, IsPlaying);
DEF_PROP_GET(AudioEmitter, bool, IsPaused);

DEF_FUNC(AudioEmitter, Play, void)(CppRef iSoundRef) {
	CppRefs::ThrowPointer<AudioEmitter>(iSoundRef)->Play();
}

DEF_FUNC(AudioEmitter, Stop, void)(CppRef iSoundRef) {
	CppRefs::ThrowPointer<AudioEmitter>(iSoundRef)->Stop();
}

DEF_FUNC(AudioEmitter, Pause, void)(CppRef iSoundRef) {
	CppRefs::ThrowPointer<AudioEmitter>(iSoundRef)->Pause();
}

DEF_FUNC(AudioEmitter, Resume, void)(CppRef iSoundRef) {
	CppRefs::ThrowPointer<AudioEmitter>(iSoundRef)->Resume();
}