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

        public Image() { Assets.AfterReloadEvent += OnAfterReload; }

        public void LoadAsset() {
            assetIdHash = assetId.GetHashCode();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if (cppRef.value == 0) {
                cppRef = Dll.Image.PushAsset(Game.gameRef, assetIdHash);
                ReloadAsset();
            }
        }

        public void ReloadAsset() {
            assetIdHash = assetId.GetHashCode();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if (cppRef.value == 0)
                throw new Exception("Asset not loaded");

            new FireYaml.Deserializer(assetId).InstanciateIAssetAsFile(this);

            var selfPath = FireYaml.AssetStore.Instance.GetAssetPath(assetId);
            var sourcePath = Path.ChangeExtension(selfPath, ext);

            Dll.Image.Init(Game.gameRef, cppRef, sourcePath, ref width, ref height);
        }

        private void OnAfterReload(int assetId, FireYaml.IAsset asset) {
            if (assetId != this.assetIdHash || asset == this)
                return;

            var image = asset as Image;

            this.assetId = image.assetId;
            this.assetIdHash = image.assetIdHash;
            this.cppRef = image.cppRef;
            this.width = image.width;
            this.height = image.height;
        }

    }
}
