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
    }

    [GUID("bad8395e-7c27-4697-ba7a-2e7556af5423")]
    public class Mesh : AssetBase<Mesh, MeshData>, IAsset, ISourceAsset, IEditorUIDrawer {

        public string ext { get => m_data.ext; protected set => m_data.ext = value; }
        [Open] private List<StaticMaterial> m_materials { get => m_data.materials; set => m_data.materials = value; }


        public Mesh() { }

        public Mesh(CppRef meshRef) {
            cppRef = meshRef;
            assetId = Assets.ReadCString(Dll.MeshAsset.assetId_get(cppRef));
        }

        /// <summary>
        /// Загружает StaticMesh из файла '.obj'.<br/>
        /// Позволяет загрузить меш, даже если для него нет ассета.<br/>
        /// Будет создан runtime ассет, которого нет в AssetStore. 
        /// </summary>
        public Mesh LoadFromFile(string path) {
            assetId = path;

            if (m_TakeLoadedInstance())
                return this;

            m_PushThisInstance();

            Dll.MeshAsset.Init(Game.gameRef, cppRef, path);

            return this;
        }

        public Mesh LoadFromAsset(string assetId) {
            this.assetId = assetId;
            LoadAsset();
            return this;
        }

        public override void ReloadAsset() {
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

        public void OnDrawUI() {
            if (m_materials != null && m_materials.Count > 0) {
                // GUI.DrawList("materials", m_materials);
            }
        }
    }

}
