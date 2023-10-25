#include "Audio.h"

#include <cassert>
#include <Dbt.h>
#include <ksmedia.h>
#include "wrl.h_d3d11_alias.h"

#include "Game.h"
#include "Scene.h"
#include "AssetStore.h"
#include "Assets.h"
#include "AudioEmitter.h"
#include "AudioAsset.h"
#include "Sound.h"
#include "CameraComponent.h"
#include "AudioListener.h"
#include "Math.h"


Audio::Audio() {
	auto flags = DirectX::AudioEngine_Default;

	m_audioEngine = new DirectX::AudioEngine(flags);
}

Audio::~Audio() {
	m_audioEngine->Suspend();
	delete m_audioEngine;
}

void Audio::Init(Game* game, Scene* scene) {
	m_game = game;
    m_scene = scene;
}

void Audio::Destroy() {
    m_DeleteSounds();
    m_DeleteAssets();
}

void Audio::Update() {
	if (m_resetAudio) {
		m_resetAudio = false;
        m_ResetAudio();
	}
    else {
        m_UpdateVolume();
        m_UpdateEmitters();

        if (!m_audioEngine->Update()) {
            if (m_audioEngine->IsCriticalError()) {
                // We lost the audio device!
                m_resetAudio = true;
            }
        }
    }
}

float Audio::volume() {
    return m_volume;
}

void Audio::volume(float value) {
    m_volume = clamp(value, 0, 1);
}

void Audio::Mute() {
    m_isMuted = true;
    m_audioEngine->SetMasterVolume(0);
}

void Audio::Unmute() {
    m_isMuted = false;

    float value = m_mainListener != nullptr ? m_mainListener->volume() : 0;
    m_audioEngine->SetMasterVolume(value);
}

void Audio::Suspend() {
	m_audioEngine->Suspend();
}

void Audio::Resume() {
	m_audioEngine->Resume();
}

void Audio::OnAudioDeviceChanged() {
    if (!m_audioEngine->IsAudioDevicePresent())
        m_resetAudio = true;
}

void Audio::AddAsset(AudioAsset* audioAsset) {
    auto iter = m_assets.insert(m_assets.end(), audioAsset);

    audioAsset->f_sceneIter = iter;
}

void Audio::RemoveAsset(AudioAsset* audioAsset) {
    if (audioAsset->f_sceneIter != m_assets.end())
        m_assets.erase(audioAsset->f_sceneIter);
}

void Audio::AddSound(Sound* sound, CsRef csRef) {
    auto iter = m_sounds.insert(m_sounds.end(), sound);

    sound->f_sceneIter = iter;
    sound->f_ref = CppRefs::Create(sound);
    sound->f_cppRef = sound->f_ref.cppRef();
    sound->f_csRef = csRef;

    if (csRef.value == 0)
        sound->f_csRef = m_game->callbacks().createSound(sound->cppRef());
}

void Audio::RemoveSound(Sound* sound) {
    sound->Stop();

    if (sound->f_sceneIter != m_sounds.end())
        m_sounds.erase(sound->f_sceneIter);

    CppRefs::Remove(sound->f_ref);
    m_game->callbacks().removeCsRef(sound->csRef());
}

void Audio::AddEmitter(AudioEmitter* emitter) {
    auto iter =  m_emitters.insert(m_emitters.end(), emitter);

    emitter->f_sceneIter = iter;
}

void Audio::RemoveEmitter(AudioEmitter* emitter) {
    m_emitters.erase(emitter->f_sceneIter);
}

void Audio::AddListener(AudioListener* listener) {
    auto iter = m_listeners.insert(m_listeners.end(), listener);

    if (m_mainListener == nullptr)
        m_mainListener = listener;

    listener->f_sceneIter = iter;
}

void Audio::RemoveListener(AudioListener* listener) {
    m_listeners.erase(listener->f_sceneIter);

    if (m_mainListener == listener) {
        m_mainListener = nullptr;

        if (m_listeners.size() > 0)
            m_mainListener = m_listeners.front();
    }
}

std::unique_ptr<DirectX::SoundEffect> Audio::LoadSound(const std::string& path) {
    std::wstring wPath(path.begin(), path.end());

    return std::make_unique<DirectX::SoundEffect>(m_audioEngine, wPath.c_str());
}

void Audio::WindowMassageHandler(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
    switch (umessage) {
    case WM_CREATE: 
        if (!m_hAudioNotify) {
            DEV_BROADCAST_DEVICEINTERFACE filter = {};
            filter.dbcc_size = sizeof(filter);
            filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
            filter.dbcc_classguid = KSCATEGORY_AUDIO;

            m_hAudioNotify = RegisterDeviceNotification(hwnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
        }
        break;
    
    case WM_CLOSE: 
        if (m_hAudioNotify) {
            UnregisterDeviceNotification(m_hAudioNotify);
            m_hAudioNotify = nullptr;
        }
        break;
    
    case WM_DEVICECHANGE:
        if (wparam == DBT_DEVICEARRIVAL || wparam == DBT_DEVICEREMOVECOMPLETE) {
            auto pDev = (PDEV_BROADCAST_HDR)lparam;
            if (pDev) {
                if (pDev->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                    auto pInter = (PDEV_BROADCAST_DEVICEINTERFACE)pDev;
                    if (pInter->dbcc_classguid == KSCATEGORY_AUDIO)
                        OnAudioDeviceChanged();
                }
            }
        }
        break;
    }
}

void Audio::m_ResetAudio() {
    if (m_audioEngine->Reset()) {
        // TODO: restart any looped sounds here
        for (auto* sound : m_sounds) {
            auto soundInstance = sound->soundInstance();
            auto state = soundInstance->GetState();

            if (state == DirectX::PLAYING && soundInstance->IsLooped())
                sound->Play();
        }
    }
}

void Audio::m_UpdateVolume() {
    float nextVolume = 0;

    if (!m_scene->isEditor() && !m_isMuted && m_mainListener != nullptr)
        nextVolume = m_mainListener->volume();
    
    if (m_lastVolume != nextVolume) {
        m_lastVolume = nextVolume;
        m_audioEngine->SetMasterVolume(nextVolume);
    }
}

void Audio::m_UpdateEmitters() {
    if (m_mainListener == nullptr)
        return;

    auto dt = m_game->deltaTime();

    m_mainListener->Apply3D(dt);

    for (auto* emitter : m_emitters)
        emitter->Apply3D(*m_mainListener->listener(), dt);
}

void Audio::m_DeleteAssets() {
    auto* assets = m_game->assets();

    auto it = m_assets.begin();
    while (it != m_assets.end()) {
        auto* asset = *it;
        it = m_assets.erase(asset->f_sceneIter);
        asset->f_sceneIter = m_assets.end();

        auto assetId = m_scene->ToSceneAssetId(asset->assetId());
        auto assetIdHash = assets->GetAssetIDHash(assetId);

        m_game->assets()->Pop(assetIdHash);
        asset->Release();
        delete asset;
    }
}

void Audio::m_DeleteSounds() {
    auto it = m_sounds.begin();
    while (it != m_sounds.end()) {
        auto* sound = *it;
        it = m_sounds.erase(sound->f_sceneIter);
        sound->f_sceneIter = m_sounds.end();

        delete sound;
    }
}