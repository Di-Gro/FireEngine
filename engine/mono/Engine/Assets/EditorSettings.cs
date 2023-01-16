using System;
using EngineDll;

namespace Engine {

    [GUID("3dfd7ca5-2750-4783-b7ef-9fced505568d", typeof(EditorSettings))]
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