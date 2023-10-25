#pragma once
#include <windows.h>
#include <xaudio2.h>
#include <list>
#include <Audio.h>

#include "wrl.h_d3d11_alias.h"
#include "Refs.h"

class Game;
class Scene;
class AudioAsset;
class Sound;
class AudioEmitter;
class AudioListener;

class Audio {
public:
	using AssetIter = std::list<AudioAsset*>::iterator;
	using SoundIter = std::list<Sound*>::iterator;
	using EmitterIter = std::list<AudioEmitter*>::iterator;
	using ListenerIter = std::list<AudioListener*>::iterator;

private:
	Game* m_game;
	Scene* m_scene;

	AudioListener* m_mainListener = nullptr;

	std::list<AudioListener*> m_listeners;
	std::list<AudioAsset*> m_assets;
	std::list<Sound*> m_sounds;
	std::list<AudioEmitter*> m_emitters;
	
	DirectX::AudioEngine* m_audioEngine;
	bool m_resetAudio = false;

	HDEVNOTIFY m_hAudioNotify = nullptr;

	bool m_isMuted = false;
	float m_volume = 1;
	float m_lastVolume = -1;

public:
	Audio();
	~Audio();

	void Init(Game* game, Scene* scene);
	void Destroy();
	void Update();
	void Suspend();
	void Resume();

	void OnAudioDeviceChanged();

	float volume();
	void volume(float value);

	bool IsMuted() { return m_isMuted; }
	void Mute();
	void Unmute();

	AudioListener* mainListener() { return m_mainListener; }

	void AddAsset(AudioAsset* audioAsset);
	void AddSound(Sound* sound, CsRef csRef = RefCs(0));
	void AddEmitter(AudioEmitter* emitter);
	void AddListener(AudioListener* listener);

	void RemoveAsset(AudioAsset* audioAsset);
	void RemoveSound(Sound* sound);
	void RemoveEmitter(AudioEmitter* emitter);
	void RemoveListener(AudioListener* listener);

	std::unique_ptr<DirectX::SoundEffect> LoadSound(const std::string& path);

	void WindowMassageHandler(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

private:
	void m_ResetAudio();
	void m_UpdateVolume();
	void m_UpdateEmitters();

	void m_DeleteAssets();
	void m_DeleteSounds();

};