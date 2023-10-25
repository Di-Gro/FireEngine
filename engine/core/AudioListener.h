#pragma once

#pragma once

#include <Audio.h>

#include "Actor.h"
#include "Audio.h"
#include "Sound.h"
#include "CSBridge.h"


class AudioListener : public Component {
	friend class Audio;

	COMPONENT(AudioListener);

private:
	Audio::ListenerIter f_sceneIter;

	DirectX::AudioListener m_listener;
	bool m_isInited = false;
	float m_preInitVolume = -1;

public:
	//void OnInit() override;
	//void OnDestroy() override;
	//void OnInitDisabled() { OnInit(); }
	//void OnDestroyDisabled() { OnDestroy(); }

	void OnActivate();
	void OnDeactivate();

	DirectX::AudioListener* listener() { return &m_listener; }

	bool IsMainListener();
	bool IsMuted();

	float volume();
	void volume(float value);
	
	void Mute();
	void Unmute();

	void Apply3D(float dt);

};
DEC_COMPONENT(AudioListener);

PROP_GET(AudioListener, bool, IsMainListener);
PROP_GET(AudioListener, bool, IsMuted);
PROP_GETSET(AudioListener, float, volume);
FUNC(AudioListener, Mute, void)(CppRef compRef);
FUNC(AudioListener, Unmute, void)(CppRef compRef);

