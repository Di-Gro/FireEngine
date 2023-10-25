#include "AudioListener.h"

#include "Scene.h"
#include "Audio.h"
#include "Math.h"


void AudioListener::OnActivate() {
	scene()->audio()->AddListener(this);

	if (!m_isInited) {
		m_isInited = true;
		if (m_preInitVolume >= 0) {
			volume(m_preInitVolume);
			m_preInitVolume = -1;
		}
	}
}

void AudioListener::OnDeactivate() {
	scene()->audio()->RemoveListener(this);
}

bool AudioListener::IsMainListener() {
	return scene()->audio()->mainListener() == this;
}

float AudioListener::volume() {
	if (!m_isInited)
		return clamp(m_preInitVolume, 0, 1);

	if (IsMainListener())
		return scene()->audio()->volume();

	return 0;
}

void AudioListener::volume(float value) {
	if (!m_isInited) {
		m_preInitVolume = value;
		return;
	}
	if (IsMainListener())
		scene()->audio()->volume(value);
}

bool AudioListener::IsMuted() { 
	if (IsMainListener())
		scene()->audio()->IsMuted();

	return true;
}

void AudioListener::Mute() {
	if (IsMainListener())
		scene()->audio()->Mute();
}

void AudioListener::Unmute() {
	if (IsMainListener())
		scene()->audio()->Unmute();
}

void AudioListener::Apply3D(float dt) {
	static X3DAUDIO_CONE s_listenerCone = {
		X3DAUDIO_PI * 5.0f / 6.0f, X3DAUDIO_PI * 11.0f / 6.0f, 1.0f, 0.75f, 0.0f, 0.25f, 0.708f, 1.0f
	};

	auto actorWpos = worldPosition();
	auto actorForward = localForward();
	auto actorUp = localUp();

	m_listener.pCone = const_cast<X3DAUDIO_CONE*>(&s_listenerCone);
	m_listener.Position = actorWpos;
	m_listener.OrientFront = actorForward;
	m_listener.OrientTop = actorUp;
	m_listener.SetVelocity(actorForward * dt);
}

DEF_COMPONENT(AudioListener, Engine.AudioListener, 0, RunMode::EditPlay) { }

DEF_PROP_GET(AudioListener, bool, IsMainListener);
DEF_PROP_GET(AudioListener, bool, IsMuted);
DEF_PROP_GETSET(AudioListener, float, volume);

DEF_FUNC(AudioListener, Mute, void)(CppRef compRef) {
	CppRefs::ThrowPointer<AudioListener>(compRef)->Mute();
}

DEF_FUNC(AudioListener, Unmute, void)(CppRef compRef) {
	CppRefs::ThrowPointer<AudioListener>(compRef)->Unmute();
}