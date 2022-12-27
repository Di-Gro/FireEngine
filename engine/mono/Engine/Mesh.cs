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

        private int m_assetIdHash;

        public StaticMesh() { } /// For IAsset //TODO: make public

        public StaticMesh(string file) {
            m_assetIdHash = file.GetHashCode();
            Dll.MeshAsset.Load(Game.meshAssetRef, m_assetIdHash);

            meshRef = Dll.MeshAsset.GetMesh(Game.meshAssetRef, m_assetIdHash);
        }

        public StaticMesh(int assetIdHash) {
            m_assetIdHash = assetIdHash;
            Dll.MeshAsset.Load(Game.meshAssetRef, m_assetIdHash);

            meshRef = Dll.MeshAsset.GetMesh(Game.meshAssetRef, m_assetIdHash);
        }

        public void LoadAsset() {
            var path = FireYaml.AssetStore.Instance.GetAssetPath(assetId);

            m_assetIdHash = path.GetHashCode();
            Dll.MeshAsset.Load(Game.meshAssetRef, m_assetIdHash);

            meshRef = Dll.MeshAsset.GetMesh(Game.meshAssetRef, m_assetIdHash);
        }

        public void ReloadAsset() {

        }

        private void OnAfterReload(int assetId, FireYaml.IAsset asset) {

        }

    }

}
