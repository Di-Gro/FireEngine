using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FireYaml {

    public class AssetStore {

        public static AssetStore Instance { get; set; }

        private Dictionary<int, string> m_typeId_typeName = new Dictionary<int, string>();
        private Dictionary<string, string> m_typeName_typeId = new Dictionary<string, string>();
        private Dictionary<string, int> m_typeName_typeId_int = new Dictionary<string, int>();


        private Dictionary<string, YamlValues> tmp_assetValues = new Dictionary<string, YamlValues>();

        private Dictionary<int, string> m_assetIds = new Dictionary<int, string>();
        private Dictionary<int, string> m_assetPaths = new Dictionary<int, string>();

        private string m_assetsPath = @"C:\Users\Dmitry\Desktop\Example\FireProject\";


        public AssetStore(bool addDefaultAssets = true) {
            AddTypeId("00000010000", typeof(Engine.Actor).FullName);
            AddTypeId("00000010003", typeof(Engine.MeshComponent).FullName);
            AddTypeId("00000010004", typeof(Engine.CameraComponent).FullName);
            AddTypeId("00000010005", typeof(Engine.TestMesh).FullName);
            AddTypeId("00000010006", typeof(UI.TestImGui).FullName);
            AddTypeId("00000010007", typeof(Engine.CSComponent).FullName);
            AddTypeId("00000010008", typeof(Engine.TestMeshBase).FullName);
            AddTypeId("00000010009", typeof(Engine.TestPrefab).FullName);
            AddTypeId("00000010010", typeof(Engine.Texture).FullName);
            AddTypeId("00000010011", typeof(Engine.Image).FullName);
            AddTypeId("00000010012", typeof(Engine.StaticMaterial).FullName);

            if (addDefaultAssets) {
                AddAssetId("TestPrefab", $"{m_assetsPath}TestPrefab.yml");
                AddAssetId("TestMesh", $"{m_assetsPath}TestMesh.yml");

                AddAssetId("MESH000001", "../../data/assets/levels/farm/meshes/House_Red.obj");
                AddAssetId("MESH000002", "../../data/assets/levels/farm/meshes/House_Purple.obj");
                AddAssetId("MESH000003", "../../data/assets/levels/farm/meshes/House_Blue.obj");
                AddAssetId("TestTexture1", $"{m_assetsPath}TestTexture1.yml");
                AddAssetId("TestImage1", $"{m_assetsPath}TestImage1.yml");
                AddAssetId("TestMaterial1", $"{m_assetsPath}TestMaterial1.yml");
            }
        }

        public string GetTypeName(string typeId) => m_typeId_typeName[typeId.GetHashCode()];
        public string GetTypeName(int typeId) => m_typeId_typeName[typeId];
        
        public void AddTypeId(string assetId, string value) {
            var typeId = assetId.GetHashCode();

            m_typeId_typeName[typeId] = value;
            m_typeName_typeId[value] = assetId;
            m_typeName_typeId_int[value] = typeId;
        }

        public void AddAssetId(string assetId, string path) {
            var assetIdInt = assetId.GetHashCode();
            m_assetIds[assetIdInt] = assetId;
            m_assetPaths[assetIdInt] = path;
        }

        public string GetAssetId(int assetId) {
            return m_assetIds[assetId];
        }

        public bool HasAssetPath(string assetId) {
            return GetAssetPath(assetId) != "";
        }

        public bool TryGetAssetIdByType(string typeName, out string assetId) {
            assetId = "";
            if (m_typeName_typeId.ContainsKey(typeName)) {
                assetId = m_typeName_typeId[typeName];
                return true;
            }
            return false;
        }

        public bool TryGetAssetIdByType(string typeName, out int assetId) {
            assetId = 0;
            if (m_typeName_typeId.ContainsKey(typeName)) {
                assetId = m_typeName_typeId_int[typeName];
                return true;
            }
            return false;
        }

        public string GetTypeIdFromAsset(int assetId) {
            var assetIdStr = GetAssetId(assetId);
            var values = GetAssetValues(assetIdStr);

            var scriptIdPath = ".file1!scriptId";
            var hasScriptId = values.HasValue(scriptIdPath);
            if (!hasScriptId )
                throw new Exception("Asset not contains .file1!scriptId");

            var typeId = values.GetValue(scriptIdPath).value;
            
            return typeId;
        }

        public YamlValues? GetAssetValues(string assetId) {
            LoadAsset(assetId);

            if (!tmp_assetValues.ContainsKey(assetId))
                return null;

            return tmp_assetValues[assetId];
        }

        public YamlValues ThrowAssetValues(string assetId) {
            LoadAsset(assetId);

            if (!tmp_assetValues.ContainsKey(assetId))
                throw new Exception($"Asset with assetId:{assetId} not exist");

            return tmp_assetValues[assetId];
        }

        public void LoadAsset(string prefabId) {
            if (tmp_assetValues.ContainsKey(prefabId))
                return;

            var path = GetAssetPath(prefabId);
            if (path == "")
                return;

            var text = File.ReadAllText(path);
            tmp_assetValues[prefabId] = new YamlValues().LoadFromText(text);
        }

        public string GetAssetPath(string assetId) {
            var assetIdInt = assetId.GetHashCode();
            if (m_assetPaths.ContainsKey(assetIdInt))
                return m_assetPaths[assetIdInt];
            
            return "";
        }

        public AssetInfo GetAssetInfo(string assetId) {
            var values = GetAssetValues(assetId);

            var assetIdPath = ".file0.assetId";
            var filesPath = ".file0.files";

            var hasAssetId = values.HasValue(assetIdPath);
            var hasfiles = values.HasValue(filesPath);

            if (!hasAssetId || !hasfiles)
                throw new Exception("Asset not contains AssetInfo in file0");

            var info = new AssetInfo();
            info.assetId = values.GetValue(assetIdPath).value;
            info.files = int.Parse(values.GetValue(filesPath).value);

            return info;
        }

        public void CreateAsset(string path, object data, string tmp_id) {
            var serializer = new Serializer(ignoreExistingIds: true, startId: 1);
            serializer.Serialize(data);

            if (!serializer.Result)
                throw new Exception($"Can not create asset: Serialization failed.");

            var values = serializer.Values;
            var assetPath = $"{m_assetsPath}\\{path}";

            values.AddValue(".file0.assetId", new YamlValue(YamlValue.Type.AssetId, tmp_id));
            values.AddValue(".file0.files", new YamlValue(YamlValue.Type.Var, $"{serializer.FilesCount}"));


            File.WriteAllText(assetPath, values.ToSortedText());

            AddAssetId(tmp_id, assetPath);
        }

        public void UpdateAsset(string assetId, object data) {
            if (!HasAssetPath(assetId))
                throw new Exception($"Asset with assetId:{assetId} not exist");

            var assetInfo = GetAssetInfo(assetId);
            var assetPath = GetAssetPath(assetId);

            var serializer = new Serializer(ignoreExistingIds: false, writeNewIds: true, startId: assetInfo.files + 1);
            serializer.Serialize(data);

            if (!serializer.Result)
                throw new Exception($"Can not update asset: Serialization failed.");

            var values = serializer.Values;

            values.AddValue(".file0.assetId", new YamlValue(YamlValue.Type.AssetId, assetInfo.assetId));
            values.AddValue(".file0.files", new YamlValue(YamlValue.Type.Var, $"{serializer.FilesCount}"));

            File.WriteAllText(assetPath, values.ToSortedText());

            tmp_assetValues.Remove(assetId);
            LoadAsset(assetId);
        }

        public void UpdateAssetValues(string assetId) {
            if (!HasAssetPath(assetId))
                throw new Exception($"Asset with assetId:{assetId} not exist");

            var assetPath = GetAssetPath(assetId);
            var values = GetAssetValues(assetId);

            File.WriteAllText(assetPath, values.ToSortedText());

            tmp_assetValues.Remove(assetId);
            LoadAsset(assetId);
        }

    }
}
