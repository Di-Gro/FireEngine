#pragma once

#include <Audio.h>

#include "CSBridge.h"

class ISound {
private:
	bool m_loop = false;

public:
	bool loop() { return m_loop; }
	void loop(bool value) { m_loop = value; }

	virtual DirectX::SoundEffectInstance* soundInstance() = 0;

	virtual bool IsStopped();
	virtual bool IsPlaying();
	virtual bool IsPaused();

	virtual void Play();
	virtual void Stop();
	virtual void Pause();
	virtual void Resume();
};


