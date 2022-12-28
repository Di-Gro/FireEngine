using System;
using System.Collections.Generic;
using System.Text;

using EngineDll;

namespace Engine {

    [Serializable]
    public class Texture : FireYaml.IFile, FireYaml.IAsset {

        [Open] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; }

        /// FireYaml.IFile ->
        [Close] public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();

        [Close] public int fileId { get; set; } = -1;

        [Close] public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 

        public Image image;
        public uint width;
        public uint height;

        [Close] public CppRef cppRef { get; private set; }

        public Texture() { 
            Assets.AfterReloadEvent += OnAfterReload;
        }

        public void LoadAsset() {
            assetIdHash = assetId.GetHashCode();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0){
                cppRef = Dll.Texture.PushAsset(Game.gameRef, assetId, assetIdHash);
                ReloadAsset();
            }
        }

        public void ReloadAsset() {
            assetIdHash = assetId.GetHashCode();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0)
                throw new Exception("Asset not loaded");

            new FireYaml.Deserializer(assetId).InstanciateIAssetAsFile(this);

            if (image == null)
                Dll.Texture.Init(Game.gameRef, cppRef, width, height);
            else 
                Dll.Texture.InitFromImage(Game.gameRef, cppRef, image.cppRef);
        }

        private void OnAfterReload(int assetId, FireYaml.IAsset asset) {
            if(assetId != this.assetIdHash || asset == this)
                return;
                
            var texture = asset as Texture;

            this.assetId = texture.assetId;
            this.assetIdHash = texture.assetIdHash;
            this.cppRef = texture.cppRef;
            this.image = texture.image;
            this.width = texture.width;
            this.height = texture.height;
        }
    }
}
