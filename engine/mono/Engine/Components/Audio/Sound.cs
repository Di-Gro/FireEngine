using EngineDll;

namespace Engine {

    [GUID("6ddb549a-e7cd-46db-ab1e-1a6d975a1208")]
    public sealed class Sound {

        public CsRef csRef { get; private set; }
        public CppRef cppRef { get; private set; }
        public bool IsDestroyed => CppLinked.GetObjectByRef(csRef) == null;

        public bool IsStopped => Dll.Sound.IsPaused_get(cppRef);
        public bool IsPlaying => Dll.Sound.IsPlaying_get(cppRef);
        public bool IsPaused => Dll.Sound.IsPaused_get(cppRef);

        public bool Loop {
            get => Dll.Sound.loop_get(cppRef);
            set => Dll.Sound.loop_set(cppRef, value);
        }

        [Close] public AudioAsset asset {
            get => m_asset;
            set {
                m_asset = value;
                Dll.Sound.SetAsset(cppRef, m_asset != null ? m_asset.cppRef : CppRef.NullRef);
            }
        }
        
        private AudioAsset m_asset = null;

        public Sound() {
            csRef = CppLinked.CreateCsRef(this);
            cppRef = Dll.Scene.CreateSound(Game.sceneRef, csRef); 
        }

        public Sound(AudioAsset _asset): this() {
            asset = _asset;
        }

        private Sound(CppRef soundRef) {
            csRef = CppLinked.CreateCsRef(this);
            cppRef = soundRef;
        }

        public void Destroy() {
            Dll.Scene.RemoveSound(cppRef);
        }

        public void Play() => Dll.Sound.Play(cppRef);
        public void Stop() => Dll.Sound.Stop(cppRef);
        public void Pause() => Dll.Sound.Pause(cppRef);
        public void Resume() => Dll.Sound.Resume(cppRef);

        public static CsRef cpp_CreateSound(CppRef cppRef) {
            return new Sound(cppRef).csRef;
        }

        public static void cpp_SetAsset(CsRef soundRef, CppRef assetRef) {
            var sound = CppLinked.GetObjectByRef(soundRef) as Sound;
            if (sound == null)
                return;

            AudioAsset asset = null;
            
            if (assetRef != CppRef.NullRef) {
                var assetId = Dll.AudioAsset.assetId_get(assetRef);

                asset = new AudioAsset();
                asset.Init(assetId, assetRef);
            }
            sound.m_asset = asset;
        }
    }
}
