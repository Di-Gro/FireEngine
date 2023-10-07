using System;
using System.Collections.Generic;
using System.Text;

using EngineDll;

namespace Engine {

    [GUID("81b31ac1-86d8-4d99-aff6-324c5f987b15", typeof(Texture))]
    public class Texture : FireYaml.IFile, FireYaml.IAsset {

        /// FireYaml.IAsset ->
        [Open] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; }
        [Close] public CppRef cppRef { get; private set; } = CppRef.NullRef;
        /// <- 
        /// FireYaml.IFile ->
        [Close] public ulong assetInstance { get; set; } = 0;

        [Close] public int fileId { get; set; } = -1;

        [Close] public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 

        public Image image;
        public uint width = 128;
        public uint height = 128;
       

        public Texture() { 
            Assets.AfterReloadEvent += OnAfterReload;
            assetInstance = FireYaml.AssetInstance.PopId();
        }

        ~Texture() { Assets.AfterReloadEvent -= OnAfterReload; }

        public void LoadAsset() {
            assetIdHash = assetId.GetAssetIDHash();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0){
                cppRef = Dll.Texture.PushAsset(Game.gameRef, assetId, assetIdHash);
                Assets.SetLoadedAsset(assetIdHash, this);
                ReloadAsset();
            }
            else {
                OnAfterReload(assetIdHash, Assets.GetLoadedAsset(assetIdHash));
            }
        }

        public void ReloadAsset() {
            assetIdHash = assetId.GetAssetIDHash();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0)
                throw new Exception("Asset not loaded");

            AssetStore.GetAssetDeserializer(assetIdHash).InstanciateToWithoutLoad(this);

            if (image == null)
                Dll.Texture.Init(Game.gameRef, cppRef, width, height);
            else 
                Dll.Texture.InitFromImage(Game.gameRef, cppRef, image.cppRef);
        }

        private void OnAfterReload(int assetId, FireYaml.IAsset asset) {
            if(assetId != this.assetIdHash || asset == this)
                return;
                
            var texture = asset as Texture;
            if (texture == null)
                throw new Exception($"Asset with assetId: '{assetId}' is not {nameof(Texture)} but {asset.GetType().Name}");

            this.assetId = texture.assetId;
            this.assetIdHash = texture.assetIdHash;
            this.cppRef = texture.cppRef;
            this.image = texture.image;
            this.width = texture.width;
            this.height = texture.height;
        }

        public void SaveAsset() {

        }
    }
}
