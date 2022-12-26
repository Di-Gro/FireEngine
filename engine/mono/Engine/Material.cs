using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;

namespace Engine {

    public enum CullMode { Front, Back, None };
    public enum FillMode { Solid, Wireframe };

    interface IMaterial {
        bool IsDynamic { get; }

        CppRef matRef { get; }
        
        string Name { get; }

        Vector3 DiffuseColor { get; }
        float Diffuse { get; }
        float Ambient { get; }
        float Specular { get; }
        float Shininess { get; }

        CullMode CullMode { get; }
        FillMode FillMode { get; }
        
        ulong Priority { get; }

    } 

    [Serializable]
    class StaticMaterial : IMaterial, FireYaml.IFile, FireYaml.IAsset {

        /// FireYaml.IFile ->
        [Close] public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();

        [Close] public int fileId { get; set; } = -1;

        [Close] public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 

        public bool IsDynamic => Dll.Material.isDynamic_get(matRef);

        [Open] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; }

        public CppRef matRef { get; protected set; }

        [Open] public string Name { get; private set; }
        
        [Open] public CullMode CullMode { get => (CullMode)Dll.Material.cullMode_get(matRef); private set => Dll.Material.cullMode_set(matRef, (int)value); }
        [Open] public FillMode FillMode { get => (FillMode)Dll.Material.fillMode_get(matRef); private set => Dll.Material.fillMode_set(matRef, (int)value); }
        [Open] public ulong Priority { get => Dll.Material.priority_get(matRef); private set => Dll.Material.priority_set(matRef, value); }

        [Open] public Vector3 DiffuseColor { get => Dll.Material.diffuseColor_get(matRef); private set => Dll.Material.diffuseColor_set(matRef, value); }
        [Open] public float Diffuse { get => Dll.Material.diffuse_get(matRef); private set => Dll.Material.diffuse_set(matRef, value); }
        [Open] public float Ambient { get => Dll.Material.ambient_get(matRef); private set => Dll.Material.ambient_set(matRef, value); }
        [Open] public float Specular { get => Dll.Material.specular_get(matRef); private set => Dll.Material.specular_set(matRef, value); }
        [Open] public float Shininess { get => Dll.Material.shininess_get(matRef); private set => Dll.Material.shininess_set(matRef, value); }

        [Open] public List<Texture> textures { get; private set; }


        public StaticMaterial() {
            Assets.AfterReloadEvent += OnAfterReload;
        }

        public StaticMaterial(CppRef cppRef) {
            Console.WriteLine("StaticMaterial(CppRef cppRef)");
            matRef = cppRef;

            byte[] buf = new byte[80];
            int writedLength = Dll.Material.name_get(matRef, buf, buf.Length);

            Name = Encoding.ASCII.GetString(buf).TrimEnd('\0');
        }

        public void LoadAsset() {

            // var path = FireYaml.AssetStore.Instance.GetAssetPath(assetId);
            // var assetHash = Dll.Assets.GetHash(Game.meshAssetRef, path);

            // matRef = Dll.Material.Create(Game.gameRef, assetHash);

            // new FireYaml.Deserializer(assetId).InstanciateIAssetAsFile(this);

        }

        public void ReloadAsset() {

        }

        private void OnAfterReload(int assetId, FireYaml.IAsset asset) {

        }

    }

    class DynamicMaterial : IMaterial {

        public bool IsDynamic => Dll.Material.isDynamic_get(matRef);

        public CppRef matRef { get; protected set; }

        public string Name { get; private set; }

        public Vector3 DiffuseColor { get => Dll.Material.diffuseColor_get(matRef); set => Dll.Material.diffuseColor_set(matRef, value); }
        public float Diffuse { get => Dll.Material.diffuse_get(matRef); set => Dll.Material.diffuse_set(matRef, value); }
        public float Ambient { get => Dll.Material.ambient_get(matRef); set => Dll.Material.ambient_set(matRef, value); }
        public float Specular { get => Dll.Material.specular_get(matRef); set => Dll.Material.specular_set(matRef, value); }
        public float Shininess { get => Dll.Material.shininess_get(matRef); set => Dll.Material.shininess_set(matRef, value); }

        public CullMode CullMode { get => (CullMode)Dll.Material.cullMode_get(matRef); set => Dll.Material.cullMode_set(matRef, (int)value); }
        public FillMode FillMode { get => (FillMode)Dll.Material.fillMode_get(matRef); set => Dll.Material.fillMode_set(matRef, (int)value); }

        public ulong Priority { get => Dll.Material.priority_get(matRef); set => Dll.Material.priority_set(matRef, value); }

        public bool IsDeleted => Ref.IsValid(matRef);


        public DynamicMaterial(IMaterial source) {
            //Console.WriteLine($"#: new DynamicMaterial(source:{source.matRef})");

            matRef = Dll.MeshAsset.CreateDynamicMaterial(Game.meshAssetRef, source.matRef);

            byte[] buf = new byte[80];
            Dll.Material.name_get(matRef, buf, buf.Length);

            Name = Encoding.ASCII.GetString(buf).TrimEnd((Char)0);
        }

        ~DynamicMaterial() {
            Console.WriteLine("#: ~DynamicMaterial()");
        }

        public void Delete() {
            Dll.MeshAsset.DeleteDynamicMaterial(Game.meshAssetRef, matRef);
        }

    }

}
