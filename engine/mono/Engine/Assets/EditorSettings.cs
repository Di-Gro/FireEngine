using System;
using EngineDll;
using FireYaml;

namespace Engine {

    [GUID("3dfd7ca5-2750-4783-b7ef-9fced505568d", typeof(EditorSettings))]
    public class EditorSettings : IFile, IAsset, IAssetEditorListener {
        /// FireYaml.IFile ->
        [Close] public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();

        [Close] public int fileId { get; set; } = -1;

        [Close] public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 
        /// FireYaml.IAsset ->
        [Open][ReadOnly] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; }
        [Close] public CppRef cppRef { get; private set; } = CppRef.NullRef;
        /// <- 

        public Scene StartupScene;

        private int m_lastSceneHash;

        public EditorSettings() {
            Assets.AssetUpdateEvent += m_OnAssetUpdate;
        }

        public EditorSettings(string assetId) {
            Assets.AssetUpdateEvent += m_OnAssetUpdate;

            this.assetId = assetId;
            assetIdHash = assetId.GetAssetIDHash();
        }

        ~EditorSettings() {
            Assets.AssetUpdateEvent -= m_OnAssetUpdate;
        }

        public void LoadAsset() {
            assetIdHash = assetId.GetAssetIDHash();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if (cppRef.value == 0) {
                cppRef = Dll.PureAsset.PushAsset(Game.gameRef, assetId, assetIdHash);
                Assets.SetLoadedAsset(this);
                ReloadAsset();
            }
            else {
                m_OnAssetUpdate(assetIdHash, Assets.GetLoadedAsset(assetIdHash));
            }
        }

        public void ReloadAsset() {
            AssetStore.GetAssetDeserializer(assetIdHash).InstanciateToWithoutLoad(this);

            m_lastSceneHash = StartupScene == null ? 0 : StartupScene.assetIdHash;
            SendToCpp();
        }

        public void SaveAsset() {
            AssetStore.WriteAsset(assetIdHash, this);
        }

        public void OnEditAsset() {
            int sceneHash = StartupScene == null ? 0 : StartupScene.assetIdHash;
            if(m_lastSceneHash != sceneHash) {
                m_lastSceneHash = sceneHash;
                SendToCpp();
            }
        }

        public void SendToCpp() {
            Dll.Game.SetEditorSettings(
                Game.gameRef,
                StartupScene != null ? StartupScene.assetId : ""
            );
        }

        private void m_OnAssetUpdate(int assetIdHash, IAsset asset) {
            if (assetIdHash != this.assetIdHash || asset == this)
                return;

            var other = asset as EditorSettings;
            if (other == null)
                throw new Exception($"Asset with assetIdHash: '{assetIdHash}' is not {nameof(EditorSettings)} but {asset.GetType().Name}");

            this.assetId = other.assetId;
            this.assetIdHash = other.assetIdHash;
            this.StartupScene = other.StartupScene;
        }

        
    }
}