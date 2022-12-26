using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;


namespace Engine {

    class Mesh  {
        public CppRef meshRef { get; protected set; }
        public int ShapeCount => Dll.Mesh4.ShapeCount(meshRef);
        public int MaxMaterialIndex => Dll.Mesh4.MaterialMaxIndex(meshRef);

        public Mesh() {}
        public Mesh(CppRef cppRef) => meshRef = cppRef;
    }

    [Serializable]
    class StaticMesh : Mesh, FireYaml.IAsset {

        [Open] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; }

        private ulong m_assetHash;

        public StaticMesh() { } /// For IAsset //TODO: make public

        public StaticMesh(string file) {
            m_assetHash = Dll.MeshAsset.CreateHash(Game.meshAssetRef, file);
            Dll.MeshAsset.Load(Game.meshAssetRef, m_assetHash);

            meshRef = Dll.MeshAsset.GetMesh(Game.meshAssetRef, m_assetHash);
        }

        public StaticMesh(ulong assetHash) {
            m_assetHash = assetHash;
            Dll.MeshAsset.Load(Game.meshAssetRef, m_assetHash);

            meshRef = Dll.MeshAsset.GetMesh(Game.meshAssetRef, m_assetHash);
        }

        public void LoadAsset() {
            var path = FireYaml.AssetStore.Instance.GetAssetPath(assetId);

            m_assetHash = Dll.MeshAsset.CreateHash(Game.meshAssetRef, path);
            Dll.MeshAsset.Load(Game.meshAssetRef, m_assetHash);

            meshRef = Dll.MeshAsset.GetMesh(Game.meshAssetRef, m_assetHash);
        }

        public void ReloadAsset() {

        }

        private void OnAfterReload(int assetId, FireYaml.IAsset asset) {

        }

    }

}
