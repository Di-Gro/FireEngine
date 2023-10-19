using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;
using FireYaml;

namespace Engine {

    public enum CullMode { Front, Back, None };
    public enum FillMode { Solid, Wireframe };

    public interface IMaterial {
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

    public class MaterialData : AssetDataBase {
        public List<Texture> textures { get; set; }
        public long texturesHash = 0;
        public MaterialProxy proxy = new MaterialProxy();

        public bool IsDynamic { get => proxy.IsDynamic; }
        public string Name { get => proxy.Name; set => proxy.Name = value; }
        public string Shader { get => proxy.Shader; set => proxy.Shader = value; }
        public CullMode CullMode { get => proxy.CullMode; set => proxy.CullMode = value; }
        public FillMode FillMode { get => proxy.FillMode; set => proxy.FillMode = value; }
        public ulong Priority { get => proxy.Priority; set => proxy.Priority = value; }
        public Vector3 DiffuseColor { get => proxy.DiffuseColor; set => proxy.DiffuseColor = value; }
        public float Diffuse { get => proxy.Diffuse; set => proxy.Diffuse = value; }
        public float Ambient { get => proxy.Ambient; set => proxy.Ambient = value; }
        public float Specular { get => proxy.Specular; set => proxy.Specular = value; }
        public float Shininess { get => proxy.Shininess; set => proxy.Shininess = value; }
    }

    [GUID("110065f7-ed95-4fca-8a09-288b7ec17500", typeof(StaticMaterial))]
    public class StaticMaterial : AssetBase<StaticMaterial, MaterialData>, IMaterial, IAsset, IAssetEditorListener {

        public bool IsDynamic { get => m_data.IsDynamic; }
        [Open][ReadOnly] public string Name { get => m_data.Name; private set => m_data.Name = value; }
        [Open] public string Shader { get => m_data.Shader; private set => m_data.Shader = value; }
        [Open] public CullMode CullMode { get => m_data.CullMode; private set => m_data.CullMode = value; }
        [Open] public FillMode FillMode { get => m_data.FillMode; private set => m_data.FillMode = value; }
        [Open] public ulong Priority { get => m_data.Priority; private set => m_data.Priority = value; }
        [Open] public Vector3 DiffuseColor { get => m_data.DiffuseColor; private set => m_data.DiffuseColor = value; }
        [Open] public float Diffuse { get => m_data.Diffuse; private set => m_data.Diffuse = value; }
        [Open] public float Ambient { get => m_data.Ambient; private set => m_data.Ambient = value; }
        [Open] public float Specular { get => m_data.Specular; private set => m_data.Specular = value; }
        [Open] public float Shininess { get => m_data.Shininess; private set => m_data.Shininess = value; }
        [Open] private List<Texture> m_textures { get => m_data.textures; set => m_data.textures = value; }


        public StaticMaterial() {
            Assets.TextureAssetUpdateEvent += m_OnTextureUpdate;
        }

        public StaticMaterial(CppRef cppRef) {
            Assets.TextureAssetUpdateEvent += m_OnTextureUpdate;

            this.cppRef = cppRef;
            assetId = Assets.ReadCString(Dll.Material.assetId_get(cppRef));
            assetIdHash = Dll.Material.assetIdHash_get(cppRef);
        }

        ~StaticMaterial() { 
            Assets.TextureAssetUpdateEvent -= m_OnTextureUpdate;
        }

        public StaticMaterial LoadFromAsset(string assetId) {
            this.assetId = assetId;
            LoadAsset();
            return this;
        }

        public override void ReloadAsset() {
            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0)
                throw new Exception("Asset not loaded");

            m_data.proxy.cppRef = cppRef;

            AssetStore.GetAssetDeserializer(assetIdHash).InstanciateToWithoutLoad(this);
         
            Dll.Material.Init(Game.gameRef, cppRef);

            m_data.texturesHash = GetAssetsListHash(m_textures);

            m_SendTexturesToCpp();
        }

        public void OnEditAsset() {
            var hash = GetAssetsListHash(m_textures);
            if (m_data.texturesHash != hash) {
                m_data.texturesHash = hash;
                m_SendTexturesToCpp();
            }
        }

        private void m_OnTextureUpdate(int texAssetIdHash) {
            if (m_textures == null)
                return;

            foreach(var texture in m_textures) {
                if (texture != null && texture.assetIdHash == texAssetIdHash) {
                    m_SendTexturesToCpp();
                    return;   
                }
            }
        }

        private void m_SendTexturesToCpp() {
            var cppRefs = new ulong[m_textures.Count];
            for (int i = 0; i < m_textures.Count; i++)
                cppRefs[i] = m_textures[i] == null ? 0 : m_textures[i].cppRef.value;

            Dll.Material.textures_set(cppRef, cppRefs, m_textures.Count);
        }

    }

    public class DynamicMaterial : IMaterial {

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
            m_proxy.cppRef = Dll.Material.CreateDynamicMaterial(Game.gameRef, source.cppRef);
        }

        public DynamicMaterial(string name, string assetId) {
            var source = new StaticMaterial().LoadFromAsset(assetId);

            m_proxy.cppRef = Dll.Material.CreateDynamicMaterial(Game.gameRef, source.cppRef);
            m_proxy.Name = name;
        }

        public void UpdateTextures() {
            var cppRefs = new ulong[textures.Count];
            for (int i = 0; i < textures.Count; i++)
                cppRefs[i] = textures[i] == null ? 0 : textures[i].cppRef.value;

            Dll.Material.textures_set(cppRef, cppRefs, textures.Count);
        }

        public void Delete() {
            Dll.Material.DeleteDynamicMaterial(Game.gameRef, cppRef);
        }

    }

    public class MaterialProxy : IMaterial {

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
        private string m_Shader = "../../engine/data/shaders/opaque_defuse_color.hlsl";
        private CullMode m_CullMode = CullMode.Back;
        private FillMode m_FillMode = FillMode.Solid;
        private ulong m_Priority = 2000;
        private Vector3 m_DiffuseColor = Vector3.One;
        private float m_Diffuse = 0.8f;
        private float m_Ambient = 0.5f;
        private float m_Specular = 0.2f;
        private float m_Shininess = 250;

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
