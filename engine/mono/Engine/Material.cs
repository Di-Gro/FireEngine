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

        CppRef cppRef { get; }
        
        string Name { get; }
        string Shader { get; }

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

        /// IFile ->
        [Close] public ulong assetInstance { get; set; } = FireYaml.AssetInstance.PopId();
        [Close] public int fileId { get; set; } = -1;
        [Close] public string prefabId { get; set; } = FireYaml.IFile.NotPrefab;
        /// <- 

        /// IAsset ->
        [Open] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; }
        /// <- 

        /// IMaterial ->
        public CppRef cppRef { get => m_proxy.cppRef; protected set => m_proxy.cppRef = value; }
        public bool IsDynamic { get => m_proxy.IsDynamic; }
        [Open] public string Name { get => m_proxy.Name; private set => m_proxy.Name = value; }
        [Open] public string Shader { get => m_proxy.Shader; private set => m_proxy.Shader = value; }
        
        [Open] public CullMode CullMode { get => m_proxy.CullMode; private set => m_proxy.CullMode = value; }
        [Open] public FillMode FillMode { get => m_proxy.FillMode; private set => m_proxy.FillMode = value; }
        [Open] public ulong Priority { get => m_proxy.Priority; private set => m_proxy.Priority = value; }
        [Open] public Vector3 DiffuseColor { get => m_proxy.DiffuseColor; private set => m_proxy.DiffuseColor = value; }
        [Open] public float Diffuse { get => m_proxy.Diffuse; private set => m_proxy.Diffuse = value; }
        [Open] public float Ambient { get => m_proxy.Ambient; private set => m_proxy.Ambient = value; }
        [Open] public float Specular { get => m_proxy.Specular; private set => m_proxy.Specular = value; }
        [Open] public float Shininess { get => m_proxy.Shininess; private set => m_proxy.Shininess = value; }
        /// <- 

        [Open] private List<Texture> m_textures { get; set; }

        private MaterialProxy m_proxy = new MaterialProxy();


        public StaticMaterial() {
            Assets.AfterReloadEvent += OnAfterReload;
        }

        public StaticMaterial(CppRef cppRef) {
            m_proxy.cppRef = cppRef;
            assetId = Assets.ReadCString(Dll.Material.assetId_get(cppRef));
            assetIdHash = Dll.Material.assetIdHash_get(cppRef);
        }

        public StaticMaterial LoadFromAsset(string assetId) {
            this.assetId = assetId;
            LoadAsset();
            return this;
        }

        public void LoadAsset() {
            assetIdHash = assetId.GetHashCode();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0){
                cppRef = Dll.Material.PushAsset(Game.gameRef, assetId, assetIdHash);
                ReloadAsset();
            }
        }

        public void ReloadAsset() {
            assetIdHash = assetId.GetHashCode();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0)
                throw new Exception("Asset not loaded");

            new FireYaml.Deserializer(assetId).InstanciateIAssetAsFile(this);

            Dll.Material.Init(Game.gameRef, cppRef);

            if(m_textures.Count > 0) {
                var cppRefs = new ulong[m_textures.Count];
                for(int i = 0; i < m_textures.Count; i++)
                    cppRefs[i] = m_textures[i].cppRef.value;

                Dll.Material.textures_set(cppRef, cppRefs, m_textures.Count);
            }
        }

        private void OnAfterReload(int assetId, FireYaml.IAsset asset) {
            if(assetId != this.assetIdHash || asset == this)
                return;
                
            var material = asset as StaticMaterial;

            this.assetId = material.assetId;
            this.assetIdHash = material.assetIdHash;
            this.m_textures = material.m_textures;
            
            this.m_proxy = new MaterialProxy();
            this.m_proxy.Name = material.m_proxy.Name;
            this.m_proxy.Shader = material.m_proxy.Shader;
            this.m_proxy.cppRef = material.m_proxy.cppRef;
        }

    }

    class DynamicMaterial : IMaterial {

        /// IMaterial ->
        public CppRef cppRef { get => m_proxy.cppRef; protected set => m_proxy.cppRef = value; }
        public bool IsDynamic { get => m_proxy.IsDynamic; }
        public string Name { get => m_proxy.Name; set => m_proxy.Name = value; }
        public string Shader { get => m_proxy.Shader; set => m_proxy.Shader = value; }
        
        public CullMode CullMode { get => m_proxy.CullMode; set => m_proxy.CullMode = value; }
        public FillMode FillMode { get => m_proxy.FillMode; set => m_proxy.FillMode = value; }
        public ulong Priority { get => m_proxy.Priority; set => m_proxy.Priority = value; }
        public Vector3 DiffuseColor { get => m_proxy.DiffuseColor; set => m_proxy.DiffuseColor = value; }
        public float Diffuse { get => m_proxy.Diffuse; set => m_proxy.Diffuse = value; }
        public float Ambient { get => m_proxy.Ambient; set => m_proxy.Ambient = value; }
        public float Specular { get => m_proxy.Specular; set => m_proxy.Specular = value; }
        public float Shininess { get => m_proxy.Shininess; set => m_proxy.Shininess = value; }
        /// <- 

        public List<Texture> textures { get; set; }

        public bool IsDeleted => Ref.IsValid(cppRef);

        private MaterialProxy m_proxy = new MaterialProxy();

        public DynamicMaterial(IMaterial source) {
            m_proxy.cppRef = Dll.MeshAsset.CreateDynamicMaterial(Game.meshAssetRef, source.cppRef);
        }

        public DynamicMaterial(string name, string assetId) {
            var source = new StaticMaterial().LoadFromAsset(assetId);

            m_proxy.cppRef = Dll.MeshAsset.CreateDynamicMaterial(Game.meshAssetRef, source.cppRef);
            m_proxy.Name = name;
        }

        public void UpdateTextures() {
            if(textures.Count > 0) {
                var cppRefs = new ulong[textures.Count];
                for(int i = 0; i < textures.Count; i++)
                    cppRefs[i] = textures[i].cppRef.value;

                Dll.Material.textures_set(cppRef, cppRefs, textures.Count);
            }
        }

        public void Delete() {
            Dll.MeshAsset.DeleteDynamicMaterial(Game.meshAssetRef, cppRef);
        }

    }

    class MaterialProxy : IMaterial {

        public bool IsDynamic { 
            get { return cppRef.value == 0 ? false : Dll.Material.isDynamic_get(cppRef); }
        }

        public CppRef cppRef { get; set; }

        public string Name {  
            get { return cppRef.value == 0 ? m_Name : m_GetName(); }
            set { m_Name = value; if(cppRef.value != 0) Dll.Material.name_set(cppRef, value); }
        }

        public string Shader {  
            get { return cppRef.value == 0 ? m_Shader : m_GetShader(); }
            set { m_Shader = value; if(cppRef.value != 0) Dll.Material.shader_set(Game.gameRef, cppRef, value); }
        }

        public CullMode CullMode {
            get { return cppRef.value == 0 ? m_CullMode : (CullMode)Dll.Material.cullMode_get(cppRef); }
            set { m_CullMode = value; if(cppRef.value != 0) Dll.Material.cullMode_set(cppRef, (int)value); }
        }

        public FillMode FillMode {
            get { return cppRef.value == 0 ? m_FillMode : (FillMode)Dll.Material.fillMode_get(cppRef); }
            set { m_FillMode = value; if(cppRef.value != 0) Dll.Material.fillMode_set(cppRef, (int)value); }
        }

        public ulong Priority { 
            get { return cppRef.value == 0 ? m_Priority : Dll.Material.priority_get(cppRef); } 
            set { m_Priority = value; if(cppRef.value != 0) Dll.Material.priority_set(cppRef, value); }
        }

        public Vector3 DiffuseColor { 
            get { return cppRef.value == 0 ? m_DiffuseColor : Dll.Material.diffuseColor_get(cppRef); }
            set { m_DiffuseColor = value; if(cppRef.value != 0) Dll.Material.diffuseColor_set(cppRef, value); }
        }

        public float Diffuse { 
            get { return cppRef.value == 0 ? m_Diffuse : Dll.Material.diffuse_get(cppRef); }
            set { m_Diffuse = value; if(cppRef.value != 0) Dll.Material.diffuse_set(cppRef, value); }
        }
        
        public float Ambient { 
            get { return cppRef.value == 0 ? m_Ambient : Dll.Material.ambient_get(cppRef); }
            set { m_Ambient = value; if(cppRef.value != 0) Dll.Material.ambient_set(cppRef, value); }
        }

        public float Specular { 
            get { return cppRef.value == 0 ? m_Specular : Dll.Material.specular_get(cppRef); }
            set { m_Specular = value; if(cppRef.value != 0) Dll.Material.specular_set(cppRef, value); }
        }

        public float Shininess { 
            get { return cppRef.value == 0 ? m_Shininess :  Dll.Material.shininess_get(cppRef); }
            set { m_Shininess = value; if(cppRef.value != 0)  Dll.Material.shininess_set(cppRef, value); }
        }

        private string m_Name = "";
        private string m_Shader = "";
        private CullMode m_CullMode;
        private FillMode m_FillMode;
        private ulong m_Priority;
        private Vector3 m_DiffuseColor;
        private float m_Diffuse;
        private float m_Ambient;
        private float m_Specular;
        private float m_Shininess;

        public void SetData() {
            if(cppRef.value == 0)
                return;

            Name = m_Name;
            Shader = m_Shader;
            CullMode = m_CullMode;
            FillMode = m_FillMode;
            Priority = m_Priority;
            DiffuseColor = m_DiffuseColor;
            Diffuse = m_Diffuse;
            Ambient = m_Ambient;
            Specular = m_Specular;
            Shininess = m_Shininess;
        }
 
        private string m_GetName() {
            var length = Dll.Material.name_length(cppRef);
            if(length <= 1)
                return "";

            byte[] buf = new byte[length];
            Dll.Material.name_get(cppRef, buf);

            var name = Encoding.ASCII.GetString(buf);
            return name;
        }

        private string m_GetShader() {
            var length = Dll.Material.shader_length(cppRef);
            if(length <= 1)
                return "";

            byte[] buf = new byte[length];
            Dll.Material.shader_get(cppRef, buf);

            var path = Encoding.ASCII.GetString(buf);
            return path;
        }

    }

}
