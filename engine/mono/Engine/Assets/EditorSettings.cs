using System;
using EngineDll;

namespace Engine {

    [Serializable]
    public class EditorSettings : FireYaml.IFile {
        /// FireYaml.IFile ->
        [Close] public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();

        [Close] public int fileId { get; set; } = -1;

        [Close] public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 

        public Scene StartupScene;

        public void UpdateInCpp() {
            Dll.Game.SetEditorSettings(
            Game.gameRef,
            StartupScene != null ? StartupScene.assetId : ""
            );
        }

    }
}