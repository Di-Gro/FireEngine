using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;


namespace Engine {

    class Mesh  {
        public CppRef cppRef { get; protected set; }
        public int ShapeCount => Dll.Mesh4.ShapeCount(cppRef);
        public int MaxMaterialIndex => Dll.Mesh4.MaterialMaxIndex(cppRef);

        public bool IsDynamic => GetType() != typeof(StaticMesh);

        public Mesh() {}
        public Mesh(CppRef cppRef) => this.cppRef = cppRef;
    }

    [Serializable]
    class StaticMesh : Mesh, FireYaml.IFile, FireYaml.IAsset {

        /// IFile ->
        [Close] public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();
        [Close] public int fileId { get; set; } = -1;
        [Close] public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 

        /// IAsset ->
        [Open] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; }
        /// <- 

        public string ext = "";
        [Open] private List<StaticMaterial> m_materials { get; set; }

        public StaticMesh() {
            Assets.AfterReloadEvent += OnAfterReload;
        }

        public StaticMesh(int assetIdHash) {
            this.assetIdHash = assetIdHash;
            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            this.assetId = Assets.ReadCString(Dll.Mesh4.assetId_get(cppRef));
        }

        public StaticMesh LoadFromFile(string path) {
            assetId = path;
            assetIdHash = assetId.GetHashCode();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0){
                cppRef = Dll.Mesh4.PushAsset(Game.gameRef, assetId, assetIdHash);
                
                Dll.Mesh4.Init(Game.gameRef, cppRef, path);
                Dll.Mesh4.assetId_set(cppRef, assetId);
            }
            return this;
        }

        public StaticMesh LoadFromAsset(string assetId) {
            this.assetId = assetId;
            LoadAsset();
            return this;
        }

        public void LoadAsset() {
            assetIdHash = assetId.GetHashCode();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0){
                cppRef = Dll.Mesh4.PushAsset(Game.gameRef, assetId, assetIdHash);
                ReloadAsset();
            }
        }

        public void ReloadAsset() {
            assetIdHash = assetId.GetHashCode();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0)
                throw new Exception("Asset not loaded");

            new FireYaml.Deserializer(assetId).InstanciateIAssetAsFile(this);

            var selfPath = FireYaml.AssetStore.Instance.GetAssetPath(assetId);
            var sourcePath = Path.ChangeExtension(selfPath, ext);

            Dll.Mesh4.Init(Game.gameRef, cppRef, sourcePath);

            if(m_materials.Count > 0) {
                var cppRefs = new ulong[m_materials.Count];
                for(int i = 0; i < m_materials.Count; i++)
                    cppRefs[i] = m_materials[i].cppRef.value;

                Dll.Mesh4.materials_set(cppRef, cppRefs, m_materials.Count);
            }
        }

        private void OnAfterReload(int assetIdHash, FireYaml.IAsset asset) {
            if(assetIdHash != this.assetIdHash || asset == this)
                return;
                
            var mesh = asset as StaticMesh;

            this.assetId = mesh.assetId;
            this.assetIdHash = mesh.assetIdHash;
            this.ext = mesh.ext;
            this.m_materials = mesh.m_materials;
        }

    }

}
