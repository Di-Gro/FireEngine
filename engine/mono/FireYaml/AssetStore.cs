using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Engine;
using EngineDll;

namespace FireYaml {

    public class AssetStore {

        public static AssetStore Instance { get; set; } = new AssetStore();
        public static string M_Default = "M_Default";

        private Dictionary<int, string> m_scriptIdHash_scriptName = new Dictionary<int, string>();
        private Dictionary<string, string> m_scriptName_scriptId = new Dictionary<string, string>();
        private Dictionary<int, string> m_scriptIdHash_scriptId = new Dictionary<int, string>();
        private Dictionary<string, int> m_scriptName_scriptIdHash = new Dictionary<string, int>();


        private Dictionary<string, YamlValues> tmp_assetValues = new Dictionary<string, YamlValues>();

        private Dictionary<int, string> m_assetIdHash_assetId = new Dictionary<int, string>();
        private Dictionary<int, string> m_assetIdHash_assetPath = new Dictionary<int, string>();
        private Dictionary<int, int> m_assetIdHash_scriptIdHash = new Dictionary<int, int>();

        private string m_assetsPath = @"..\..\Example\FireProject\";
        private uint m_nextAssetId = 16;

        public AssetStore(bool addDefaultAssets = true) {
            AddScriptId("00000010000", typeof(Engine.Actor).FullName);
            AddScriptId("00000010003", typeof(Engine.MeshComponent).FullName);
            AddScriptId("00000010004", typeof(Engine.CameraComponent).FullName);
            AddScriptId("00000010005", typeof(Engine.TestMesh).FullName);
            AddScriptId("00000010006", typeof(UI.TestImGui).FullName);
            AddScriptId("00000010007", typeof(Engine.CSComponent).FullName);
            AddScriptId("00000010008", typeof(Engine.TestMeshBase).FullName);
            AddScriptId("00000010009", typeof(Engine.TestPrefab).FullName);
            AddScriptId("00000010010", typeof(Engine.Texture).FullName);
            AddScriptId("00000010011", typeof(Engine.Image).FullName);
            AddScriptId("00000010012", typeof(Engine.StaticMaterial).FullName);
            AddScriptId("00000010013", typeof(Engine.StaticMesh).FullName);
            AddScriptId("00000010014", typeof(Engine.Scene).FullName);
            AddScriptId("00000010015", typeof(Engine.Prefab).FullName);
            AddScriptId("00000010016", typeof(Engine.Component).FullName);

            if (addDefaultAssets) {
                AddAssetId("TestPrefab", $"{m_assetsPath}TestPrefab.yml");
                AddAssetId("TestMesh", $"{m_assetsPath}TestMesh.yml");
                AddAssetId("TestMeshPrefab", "../../data/assets/new/TestMeshPrefab.yml");

                AddAssetId("MESH000001", "../../data/assets/new/House_Red.yml");
                AddAssetId("MESH000002", "../../data/assets/new/House_Purple.yml");
                AddAssetId("MESH000003", "../../data/assets/new/House_Blue.yml");
                AddAssetId("IMG0000004", "../../data/assets/new/Gradients.yml");
                AddAssetId("TEX0000005", "../../data/assets/new/Texture.yml");
                AddAssetId("MAT0000006", "../../data/assets/new/Material.yml");

                AddAssetId("TestTexture1", $"{m_assetsPath}TestTexture1.yml");
                AddAssetId("TestImage1", $"{m_assetsPath}TestImage1.yml");
                AddAssetId("TestMaterial1", $"{m_assetsPath}TestMaterial1.yml");
                AddAssetId("TestMesh1", $"{m_assetsPath}TestMesh1.yml");
                AddAssetId("M_Default", $"{m_assetsPath}M_Default.yml");
                AddAssetId("TestPrefab2", $"{m_assetsPath}TestPrefab2.yml");
            }
        }

        public string GetTypeFullName(string scriptId) => m_scriptIdHash_scriptName[scriptId.GetHashCode()];
        public string GetTypeFullName(int scriptIdHash) => m_scriptIdHash_scriptName[scriptIdHash];
        public bool HasTypeName(string typeName) => m_scriptName_scriptId.ContainsKey(typeName);
        public string GetScriptIdByTypeName(string typeName) => m_scriptName_scriptId[typeName];
        
        public void AddScriptId(string scriptId, string scriptName) {
            var scriptIdHash = scriptId.GetHashCode();

            m_scriptIdHash_scriptId[scriptIdHash] = scriptId;
            m_scriptIdHash_scriptName[scriptIdHash] = scriptName;
            m_scriptName_scriptId[scriptName] = scriptId;
            m_scriptName_scriptIdHash[scriptName] = scriptIdHash;
        }

        public void AddAssetId(string assetId, string path) {
            var assetIdInt = assetId.GetHashCode();
            m_assetIdHash_assetId[assetIdInt] = assetId;
            m_assetIdHash_assetPath[assetIdInt] = path;
        }

        public string GetAssetId(int assetId) {
            return m_assetIdHash_assetId[assetId];
        }

        public bool HasAssetPath(string assetId) {
            return GetAssetPath(assetId) != "";
        }

        public bool TryGetAssetIdByType(string typeName, out string assetId) {
            assetId = "";
            if (m_scriptName_scriptId.ContainsKey(typeName)) {
                assetId = m_scriptName_scriptId[typeName];
                return true;
            }
            return false;
        }

        public bool TryGetAssetIdByType(string typeName, out int assetId) {
            assetId = 0;
            if (m_scriptName_scriptId.ContainsKey(typeName)) {
                assetId = m_scriptName_scriptIdHash[typeName];
                return true;
            }
            return false;
        }

        public string GetAssetScriptId(int assetIdHash) {
            var scritpIdHash = m_assetIdHash_scriptIdHash[assetIdHash];
            var scriptId = m_scriptIdHash_scriptId[scritpIdHash];
            return scriptId;

            // var assetId = GetAssetId(assetIdHash);
            // var values = GetAssetValues(assetId);

            // var scriptIdPath = ".file1!scriptId";
            // var hasScriptId = values.HasValue(scriptIdPath);
            // if (!hasScriptId )
            //     throw new Exception("Asset not contains .file1!scriptId");

            // var typeId = values.GetValue(scriptIdPath).value;

            // return typeId;
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

        public void LoadAsset(string assetId) {
            if (tmp_assetValues.ContainsKey(assetId))
                return;

            var path = GetAssetPath(assetId);
            if (path == "")
                return;

            var text = File.ReadAllText(path);
            var values = new YamlValues().LoadFromText(text);
            
            var scriptIdPath = ".file1!scriptId";
            var scriptId = values.GetValue(scriptIdPath).value;
            var scriptIdHash = scriptId.GetHashCode();
            var assetIdHash = assetId.GetHashCode();

            tmp_assetValues[assetId] = values;
            m_assetIdHash_scriptIdHash[assetIdHash] = scriptIdHash;
        }

        public void ReloadAsset(string prefabId) {
            if (tmp_assetValues.ContainsKey(prefabId)) {
                tmp_assetValues.Remove(prefabId);
                LoadAsset(prefabId);
            }
        }

        public string GetAssetPath(string assetId) {
            var assetIdInt = assetId.GetHashCode();
            if (m_assetIdHash_assetPath.ContainsKey(assetIdInt))
                return m_assetIdHash_assetPath[assetIdInt];
            
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
            ReloadAsset(tmp_id);
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

        public static string CreateAssetId() {
            var id = AssetStore.Instance.m_nextAssetId++;
            var idStr = id.ToString();
            var nullCount = 10 - idStr.Length;
            var assetId = new string('0', nullCount) + idStr;
            return assetId;
        }

        public static uint CreateAssetIdInt() {
            var id = AssetStore.Instance.m_nextAssetId++;
            return id;
        }

        public static void UpdateTypesInCpp() {
            var typeNames = new List<string>();
            var assembly = Assembly.GetAssembly(typeof(AssetStore));

            var types = assembly.GetTypes();
            foreach(var type in types) {
                if (Serializer.IsComponent(type) || Serializer.IsAsset(type) || Serializer.IsFile(type)) { 
                    typeNames.Add(type.FullName);
                    if(!Instance.HasTypeName(type.FullName))
                        Instance.AddScriptId(CreateAssetId(), type.FullName);
                }
            }
            typeNames.Sort();

            Dll.AssetStore.ClearTypes(Game.gameRef);
            Dll.AssetStore.ClearAssetTypes(Game.gameRef);

            foreach (var typeName in typeNames) {
                var type = Type.GetType(typeName);
                var typeId = Instance.GetScriptIdByTypeName(type.FullName);
                var typeIdHash = typeId.GetHashCode();

                Dll.AssetStore.SetType(Game.gameRef, typeIdHash, type.FullName, type.Name);

                if(Serializer.IsUserComponent(type))
                    Dll.AssetStore.AddComponent(Game.gameRef, typeIdHash);
                
                if(Serializer.IsAsset(type))
                    Dll.AssetStore.AddAssetType(Game.gameRef, typeIdHash);
            }
        }

        public static void UpdateAssetsInCpp() {
            Dll.AssetStore.ClearAssets(Game.gameRef);

            var store = Instance;

            var actorId = Instance.GetScriptIdByTypeName(typeof(Engine.Actor).FullName);
            var prefabId = Instance.GetScriptIdByTypeName(typeof(Engine.Prefab).FullName);

            foreach(var assetIdHash in Instance.m_assetIdHash_assetId.Keys) {
                var assetId = Instance.GetAssetId(assetIdHash);
                var scriptId = m_ReadScriptId(assetIdHash);
                if (scriptId == actorId)
                    scriptId = prefabId;
                
                var scriptIdHash = scriptId.GetHashCode();
                var typeFullName = Instance.m_scriptIdHash_scriptName[scriptIdHash];
                var typeName = Type.GetType(typeFullName).Name;
                var assetName = m_ReadAssetName(assetIdHash);

                Instance.m_assetIdHash_scriptIdHash[assetIdHash] =  scriptIdHash;
                Dll.AssetStore.AddAsset(Game.gameRef, scriptIdHash, assetIdHash, assetName);
            }
        }

        private static string m_ReadScriptId(int assetIdHash){
            var path = Instance.m_assetIdHash_assetPath[assetIdHash];

            var lines = File.ReadLines(path);
            var scriptIdPath = ".file1!scriptId";

            foreach(var line in lines) {
                if(line.StartsWith(scriptIdPath)) {
                    var scriptId = new YamlValues().LoadFromText(line).GetValue(scriptIdPath).value;
                    return scriptId;
                }
            }
            return "";
        }

        private static string m_ReadAssetName(int assetIdHash) {
            var path = Instance.m_assetIdHash_assetPath[assetIdHash];
            var fileName = Path.GetFileNameWithoutExtension(path);
            return fileName;
        }

        /// <param name="objRef">From object</param>
        /// <param name="typeIdHash">To type</param>
        /// <returns></returns>
        public static bool IsAssignable(CsRef objRef, int typeIdHash) {
            var obj = CppLinked.GetObjectByRef(objRef);
            if (obj == null)
                return false;

            var fullName = Instance.GetTypeFullName(typeIdHash);
            var type = Type.GetType(fullName);

            var result = type.IsAssignableFrom(obj.GetType());
            return result;
        }



    }
}
