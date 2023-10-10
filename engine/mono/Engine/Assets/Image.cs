using System;
using System.IO;
using System.Collections.Generic;
using System.Text;

using EngineDll;
using FireYaml;

namespace Engine {

    [GUID("37da25c5-f1ed-4976-b924-0bb31816735d", typeof(Image))]
    public class Image : IFile, IAsset, ISourceAsset {

        /// IAsset ->
        [Open][ReadOnly] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; }
        [Close] public CppRef cppRef { get; private set; } = CppRef.NullRef;
        /// <- 
        /// ISourceAsset ->
        public string ext { get; set; } = "";
        /// <- 
        /// IFile ->
        [Close] public ulong assetInstance { get; set; } = 0;

        [Close] public int fileId { get; set; } = -1;

        [Close] public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 
       

        [Close] public int width;
        [Close] public int height;

        public Image() {
            Assets.AssetUpdateEvent += OnAfterReload;
            assetInstance = FireYaml.AssetInstance.PopId();
        }

        ~Image() { Assets.AssetUpdateEvent -= OnAfterReload; }

        public void LoadAsset() {
            assetIdHash = assetId.GetAssetIDHash();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if (cppRef.value == 0) {
                cppRef = Dll.Image.PushAsset(Game.gameRef, assetId, assetIdHash);
                Assets.SetLoadedAsset(assetIdHash, this);
                ReloadAsset();
            } else {
                OnAfterReload(assetIdHash, Assets.GetLoadedAsset(assetIdHash));
            }
        }

        public void ReloadAsset() {
            assetIdHash = assetId.GetAssetIDHash();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if (cppRef.value == 0)
                throw new Exception("Asset not loaded");

            AssetStore.GetAssetDeserializer(assetIdHash).InstanciateToWithoutLoad(this);

            var selfPath = AssetStore.GetAssetPath(assetIdHash);
            var sourcePath = Path.ChangeExtension(selfPath, ext);

            Dll.Image.Init(Game.gameRef, cppRef, sourcePath, ref width, ref height);
        }

        private void OnAfterReload(int assetIdHash, FireYaml.IAsset asset) {
            if (assetIdHash != this.assetIdHash || asset == this)
                return;

            var image = asset as Image;
            if (image == null)
                throw new Exception($"Asset with assetId: '{assetId}' is not {nameof(Image)} but {asset.GetType().Name}");

            this.assetId = image.assetId;
            this.assetIdHash = image.assetIdHash;
            this.cppRef = image.cppRef;
            this.width = image.width;
            this.height = image.height;
        }

        public void SaveAsset() {
            AssetStore.WriteAsset(assetIdHash, this);
        }
        
    }
}
