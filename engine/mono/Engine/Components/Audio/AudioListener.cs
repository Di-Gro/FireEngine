using System;
using System.Collections.Generic;

using EngineDll;

namespace Engine {

    [GUID("2c68b012-8d8e-4dee-b6b8-dcecd3a08060")]
    public sealed class AudioListener : CppComponent {
        public bool IsMainListener() => Dll.AudioListener.IsMainListener_get(cppRef);
        public bool IsMuted() => Dll.AudioListener.IsMuted_get(cppRef);

        [Open] private EditorButton bt_mute = new EditorButton(nameof(Mute));
        [Open] private EditorButton bt_unmute = new EditorButton(nameof(Unmute));

        [Range(0f, 1f)]
        public float Volume { 
            get => Dll.AudioListener.volume_get(cppRef); 
            set => Dll.AudioListener.volume_set(cppRef, value);
        }

        public void Mute() => Dll.AudioListener.Mute(cppRef);
        public void Unmute() => Dll.AudioListener.Unmute(cppRef);

        public override CppObjectInfo CppConstructor() {
            return Dll.AudioListener.Create(csRef);
        }
        
    }
}
