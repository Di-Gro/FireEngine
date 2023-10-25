using System;
using System.Collections.Generic;

using EngineDll;

namespace Engine {

    [GUID("33a9292f-6872-4b8a-b5b1-1bc4f6f4df72")]
    public sealed class AudioEmitter : CppComponent {
    
        public bool IsStopped => Dll.AudioEmitter.IsPaused_get(cppRef);
        public bool IsPlaying => Dll.AudioEmitter.IsPlaying_get(cppRef);
        public bool IsPaused => Dll.AudioEmitter.IsPaused_get(cppRef);

        public bool Loop {
            get => Dll.AudioEmitter.loop_get(cppRef);
            set => Dll.AudioEmitter.loop_set(cppRef, value);
        }

        [Range(0f, 20)] 
        public float CurveDistanceScaler { get => prop_CurveDistanceScaler.value; set => prop_CurveDistanceScaler.value = value; }
        
        [Range(0f, 1000)] 
        public float DopplerScaler { get => prop_DopplerScaler.value; set => prop_DopplerScaler.value = value; }
        
        [Range(0f, 1000)] 
        public float InnerRadius { get => prop_InnerRadius.value; set => prop_InnerRadius.value = value; }

        public bool run = false;

        // [Close]
        public Sound sound {
            get => m_sound;
            set {
                m_sound = value;
                Dll.AudioEmitter.SetSound(cppRef, m_sound == null ? CppRef.NullRef : m_sound.cppRef);
            }
        }

        private Sound m_sound = null;
        private Prop<float> prop_CurveDistanceScaler = new Prop<float>(0);
        private Prop<float> prop_DopplerScaler = new Prop<float>(1);
        private Prop<float> prop_InnerRadius = new Prop<float>(2);

        public void Play() => Dll.AudioEmitter.Play(cppRef);
        public void Stop() => Dll.AudioEmitter.Stop(cppRef);
        public void Pause() => Dll.AudioEmitter.Pause(cppRef);
        public void Resume() => Dll.AudioEmitter.Resume(cppRef);

        public override void OnUpdate() {
            if(run) {
                run = false;
                Play();
            }
        }

        public override CppObjectInfo CppConstructor() {
            return Dll.AudioEmitter.Create(csRef);
        }

        public static void cpp_SetSound(CsRef emitterRef, CsRef soundRef) {
            var emitter = GetObjectByRef(emitterRef) as AudioEmitter;
            if (emitter == null)
                return;

            if (soundRef != CsRef.NullRef)
                emitter.m_sound = GetObjectByRef(soundRef) as Sound;
            else
                emitter.m_sound = null;
        }
    }
}
