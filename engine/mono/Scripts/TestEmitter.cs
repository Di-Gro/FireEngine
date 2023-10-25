using System;
using System.Collections.Generic;
using System.Security.Cryptography;
using Engine;

[GUID("7757cd16-db34-4221-887d-02a928d4100c")]
public class TestEmitter : CSComponent {

    public AudioEmitter emitter;

    public AudioAsset asset;
    public Actor target;

    private Sound m_sound;

    public EditorButton bt_1 = new EditorButton(nameof(AddListener));
    public EditorButton bt_2 = new EditorButton(nameof(ResetSound));
    public EditorButton bt_3 = new EditorButton(nameof(OnPlayClick), "Play");
    public EditorButton bt_4 = new EditorButton(nameof(OnStopClick), "Stop");
    public EditorButton bt_5 = new EditorButton(nameof(OnPauseClick), "Pause");
    public EditorButton bt_6 = new EditorButton(nameof(OnResumeClick), "Resume");

    public void AddListener() {
        if (target == null)
            return;

        var listener = target.AddComponent<AudioListener>();
        listener.Volume = 0.384f;
    }

    public void ResetSound() {
        emitter.sound = new Sound(asset);
        // emitter.Play();
    }

    public void OnPlayClick() => emitter.Play();
    public void OnStopClick() => emitter.Stop();
    public void OnPauseClick() => emitter.Pause();
    public void OnResumeClick() => emitter.Resume();

    public override void OnInit() {

    }

    public override void OnStart() {
        m_sound = new Sound();
        m_sound.asset = asset;

        emitter.sound = m_sound;
        emitter.Play();
    }

    public override void OnUpdate() {
    }

    public override void OnDestroy() {
        if (!emitter.IsDestroyed)
            emitter.sound = null;

        if (!m_sound.IsDestroyed)
            m_sound.Destroy();
    }

}