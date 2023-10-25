using System;
using System.IO;

using EngineDll;
using FireYaml;

namespace Engine {

    [GUID("a278496e-d6e6-499b-9b81-b3be5a04be05")]
    public sealed class AudioAsset : AssetBase<AudioAsset, AssetDataBase>, IAsset {

        public string ext { get; private set; } = "wav";

        public override void Init(string _assetId, CppRef _cppRef) {
            if (IsInited)
                return;

            var tmpAssetId = Scene.ToSceneAssetId(Game.sceneRef, _assetId);

            IsInited = true;
            assetId = tmpAssetId;
            cppRef = _cppRef;

            if (!m_TakeLoadedInstance()) {
                m_PushThisInstance();
                if (cppRef == CppRef.NullRef)
                    m_PushCppAsset();
            }
            assetId = _assetId;
            Dll.AudioAsset.assetId_set(cppRef, assetId);
            Dll.AudioAsset.assetIdHash_set(cppRef, assetIdHash);
        }

        public AudioAsset LoadFromFile(string path) {
            Init(path, CppRef.NullRef);
            LogLoad();

            if (m_isNewInstance) {
                m_isNewInstance = false;

                LogReload();

                AssetStore.AddRuntimeAsset(assetIdHash);
                Dll.AudioAsset.Init(Game.sceneRef, cppRef);
                Dll.AudioAsset.Load(cppRef, path);
            }
            return this;
        }
        
        public override void ReloadAsset() {
            LogReload();
            
            if (Game.sceneRef == CppRef.NullRef)
                throw new Exception("Game.sceneRef == CppRef.NullRef");

            var lastAssetId = assetId;
            var lastAssetIdHash = assetIdHash;
            var tmpAssetId = Scene.ToSceneAssetId(Game.sceneRef, assetId);
            
            assetId = tmpAssetId;

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if (cppRef.value == 0)
                throw new Exception("Asset not loaded");

            AssetStore.GetAssetDeserializer(lastAssetIdHash).InstanciateToWithoutLoad(this);

            var selfPath = AssetStore.GetAssetPath(lastAssetIdHash);
            var sourcePath = Path.ChangeExtension(selfPath, ext);

            Dll.AudioAsset.Init(Game.sceneRef, cppRef);
            Dll.AudioAsset.Load(cppRef, sourcePath);

            assetId = lastAssetId;
        }
    }
}
