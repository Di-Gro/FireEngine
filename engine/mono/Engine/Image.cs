using System;
using System.IO;
using System.Collections.Generic;
using System.Text;

using EngineDll;

namespace Engine {

    [Serializable]
    public class Image : FireYaml.IFile, FireYaml.IAsset {

        [Open] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; }

        /// FireYaml.IFile ->
        [Close] public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();

        [Close] public int fileId { get; set; } = -1;

        [Close] public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 

        public string ext = "";

        [Close] public int width;
        [Close] public int height;

        [Close] public CppRef cppRef { get; private set; }

        public Image() { } /// For FireYaml.IAsset

        public void LoadAsset() {
            new FireYaml.Deserializer(assetId).InstanciateIAssetAsFile(this);

            var selfPath = FireYaml.AssetStore.Instance.GetAssetPath(assetId);
            var sourcePath = Path.ChangeExtension(selfPath, ext);
            
            cppRef = Dll.ImageAsset.Load(Game.gameRef, sourcePath, ref width, ref height);
        }

        public void ReloadAsset() {

        }

        private void OnAfterReload(int assetId, FireYaml.IAsset asset) {

        }

    }
}
