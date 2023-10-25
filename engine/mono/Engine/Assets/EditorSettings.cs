using System;
using EngineDll;
using FireYaml;

namespace Engine {

    public class EditorSettingsData : AssetDataBase {
        public Scene StartupScene;

        public int lastSceneHash;
    }

    [CsOnly]
    [GUID("3dfd7ca5-2750-4783-b7ef-9fced505568d")]
    public class EditorSettings : AssetBase<EditorSettings, EditorSettingsData>, IAsset, IAssetEditorListener
    {
        public Scene StartupScene { get => m_data.StartupScene; set => m_data.StartupScene = value; }

        public EditorSettings() { }

        public EditorSettings(string assetId) {
            Init(assetId, CppRef.NullRef);
        }

        public override void ReloadAsset() {
            AssetStore.GetAssetDeserializer(assetIdHash).InstanciateToWithoutLoad(this);

            m_data.lastSceneHash = StartupScene == null ? 0 : StartupScene.assetIdHash;
            SendToCpp();
        }

        public void OnEditAsset() {
            int sceneHash = StartupScene == null ? 0 : StartupScene.assetIdHash;
            if(m_data.lastSceneHash != sceneHash) {
                m_data.lastSceneHash = sceneHash;
                SendToCpp();
            }
        }

        public void SendToCpp() {
            Dll.Game.SetEditorSettings(
                Game.gameRef,
                StartupScene != null ? StartupScene.assetId : ""
            );
        }
        
    }
}