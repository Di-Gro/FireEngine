#include "ISound.h"

#include "AudioEmitter.h"

bool ISound::IsStopped() {
	if (soundInstance() == nullptr)
		return true;

	return soundInstance()->GetState() == DirectX::STOPPED;
}

bool ISound::IsPaused() {
	if (soundInstance() == nullptr)
		return false;

	return soundInstance()->GetState() == DirectX::PAUSED;
}

bool ISound::IsPlaying() {
	if (soundInstance() == nullptr)
		return false;

	return soundInstance()->GetState() == DirectX::PLAYING;
}

void ISound::Play() {
	if (soundInstance() != nullptr)
		soundInstance()->Play(loop());
}

void ISound::Stop() {
	if (soundInstance() != nullptr)
		soundInstance()->Stop();
}

void ISound::Pause() {
	if (soundInstance() != nullptr)
		soundInstance()->Pause();
}

void ISound::Resume() {
	if (soundInstance() != nullptr)
		soundInstance()->Resume();
}

