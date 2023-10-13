using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;
using FireYaml;

namespace Engine {

    public class Mesh  {
        public CppRef cppRef { get; protected set; } = CppRef.NullRef;
        public int ShapeCount => Dll.MeshAsset.ShapeCount(cppRef);
        public int MaxMaterialIndex => Dll.MeshAsset.MaterialMaxIndex(cppRef);

        public bool IsDynamic => GetType() != typeof(StaticMesh);

        public Mesh() {}
        public Mesh(CppRef cppRef) => this.cppRef = cppRef;
    }

    [GUID("bad8395e-7c27-4697-ba7a-2e7556af5423", typeof(StaticMesh))]
    public class StaticMesh : Mesh, IFile, IAsset, ISourceAsset, IEditorUIDrawer {
        /// IAsset ->
        [Open][ReadOnly] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; }
        /// ISourceAsset ->
        public string ext { get; set; } = "";
        /// <- 
        /// CppRef cppRef -> Mesh
        /// <- 
        /// IFile ->
        [Close] public ulong assetInstance { get; set; } = 0;
        [Close] public int fileId { get; set; } = -1;
        [Close] public string prefabId { get; set; } = IFile.NotPrefab;
        /// <- 

        [Open] private List<StaticMaterial> m_materials { get; set; }

        public StaticMesh() {
            Assets.AssetUpdateEvent += OnAssetUpdate;
            assetInstance = AssetInstance.PopId();
        }

        public StaticMesh(int assetIdHash) {
            this.assetIdHash = assetIdHash;
            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            this.assetId = Assets.ReadCString(Dll.MeshAsset.assetId_get(cppRef));
        }

        ~StaticMesh() { Assets.AssetUpdateEvent -= OnAssetUpdate; }

        public StaticMesh LoadFromFile(string path) {
            assetId = path;
            assetIdHash = assetId.GetAssetIDHash();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0){
                cppRef = Dll.MeshAsset.PushAsset(Game.gameRef, assetId, assetIdHash);
                
                Dll.MeshAsset.Init(Game.gameRef, cppRef, path);
                Dll.MeshAsset.assetId_set(cppRef, assetId);
                Assets.SetLoadedAsset(assetIdHash, this);
            }
            else {
                OnAssetUpdate(assetIdHash, Assets.GetLoadedAsset(assetIdHash));
            }
            return this;
        }

        public StaticMesh LoadFromAsset(string assetId) {
            this.assetId = assetId;
            LoadAsset();
            return this;
        }

        public void LoadAsset() {
            assetIdHash = assetId.GetAssetIDHash();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0){
                cppRef = Dll.MeshAsset.PushAsset(Game.gameRef, assetId, assetIdHash);

                Assets.SetLoadedAsset(assetIdHash, this);
                ReloadAsset();
            }
            else {
                OnAssetUpdate(assetIdHash, Assets.GetLoadedAsset(assetIdHash));
            }
        }

        public void ReloadAsset() {
            assetIdHash = assetId.GetAssetIDHash();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0)
                throw new Exception("Asset not loaded");

            AssetStore.GetAssetDeserializer(assetIdHash).InstanciateToWithoutLoad(this);

            var selfPath = AssetStore.GetAssetPath(assetIdHash);
            var sourcePath = Path.ChangeExtension(selfPath, ext);

            Dll.MeshAsset.Init(Game.gameRef, cppRef, sourcePath);

            if(m_materials.Count > 0) {
                var cppRefs = new ulong[m_materials.Count];
                for(int i = 0; i < m_materials.Count; i++)
                    cppRefs[i] = m_materials[i].cppRef.value;

                Dll.MeshAsset.materials_set(cppRef, cppRefs, m_materials.Count);
            }
        }

        private void OnAssetUpdate(int assetIdHash, FireYaml.IAsset asset) {
            if(assetIdHash != this.assetIdHash || asset == this)
                return;
                
            var mesh = asset as StaticMesh;
            if (mesh == null)
                throw new Exception($"Asset with assetId: '{assetId}' is not {nameof(StaticMesh)} but {asset.GetType().Name}");

            this.assetId = mesh.assetId;
            this.assetIdHash = mesh.assetIdHash;
            this.ext = mesh.ext;
            this.m_materials = mesh.m_materials;
        }

        public void SaveAsset() {
            AssetStore.WriteAsset(assetIdHash, this);
        }

        public void OnDrawUI() {
            if (m_materials != null && m_materials.Count > 0) {
                // GUI.DrawList("materials", m_materials);
            }
        }
    }

}
