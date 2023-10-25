using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;
using FireYaml;

namespace Engine {
    public class MeshData : AssetDataBase {
        public string ext = "";

        public List<StaticMaterial> materials = new List<StaticMaterial>();

        public int materialsHash;
    }

    [GUID("bad8395e-7c27-4697-ba7a-2e7556af5423")]
    public class Mesh : AssetBase<Mesh, MeshData>, IAsset, ISourceAsset, IAssetEditorListener {

        public string ext { get => m_data.ext; protected set => m_data.ext = value; }
        [Open] private List<StaticMaterial> m_materials { get => m_data.materials; set => m_data.materials = value; }


        public Mesh() { }

        public Mesh(CppRef meshRef) {
            cppRef = meshRef;
            assetId = Dll.MeshAsset.assetId_get(meshRef);

            Init(assetId, cppRef);
        }

        /// <summary>
        /// Загружает StaticMesh из файла '.obj'.<br/>
        /// Позволяет загрузить меш, даже если для него нет ассета.<br/>
        /// Будет создан runtime ассет, которого нет в AssetStore. 
        /// </summary>
        public Mesh LoadFromFile(string path) {
            Init(path, CppRef.NullRef);
            LogLoad();

            if (m_isNewInstance) {
                m_isNewInstance = false;

                LogReload();

                AssetStore.AddRuntimeAsset(assetIdHash);
                Dll.MeshAsset.Init(Game.gameRef, cppRef, path);
                m_data.materialsHash = GetAssetsListHash(m_materials);
            }
            return this;
        }

        public override void LoadAsset() {
            if (IsPath(assetId))
                LoadFromFile(assetId);
            else
                base.LoadAsset();
        }

        public override void ReloadAsset() {
            LogReload();
            
            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0)
                throw new Exception("Asset not loaded");

            AssetStore.GetAssetDeserializer(assetIdHash).InstanciateToWithoutLoad(this);

            var selfPath = AssetStore.GetAssetPath(assetIdHash);
            var sourcePath = Path.ChangeExtension(selfPath, ext);

            Dll.MeshAsset.Init(Game.gameRef, cppRef, sourcePath);
            m_data.materialsHash = GetAssetsListHash(m_materials);

            m_SendMaterialsToCpp();
        }

        public void OnEditAsset() {
            var hash = GetAssetsListHash(m_materials);
            if (m_data.materialsHash != hash) {
                m_data.materialsHash = hash;
                m_SendMaterialsToCpp();
            }
        }

        private void m_SendMaterialsToCpp(){
            var cppRefs = new ulong[m_materials.Count];
            for (int i = 0; i < m_materials.Count; i++)
                cppRefs[i] = m_materials[i] == null ? 0 : m_materials[i].cppRef.value;

            Dll.MeshAsset.materials_set(cppRef, cppRefs, m_materials.Count);
        }

        public static bool IsPath(string value) {
            return value.Contains('/') || value.Contains('\\') || value.Contains('.');
        }

    }

}
